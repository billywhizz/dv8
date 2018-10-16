const { start, stop } = require('./lib/meter.js')
const { Socket, TCP, UNIX } = module('socket', {})
const { printStats } = require('./lib/util.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})

const READ_BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * READ_BUFFER_SIZE
const contexts = {}

function createContext(fd) {
    const b = new Buffer()
    b.alloc(READ_BUFFER_SIZE)
    const context = { fd, in: b, out: b }
    contexts[fd] = context
    stdin.setup(fd, b, b)
    return context
}

function onConnect(fd) {
    let context = contexts[fd]
    if (!context) {
        context = createContext(fd)
    }
    const stdout = context.stdout = new TTY(1, () => {
        stop(stdout)
        printStats(stdout, 'out')
    }, () => stdin.resume(), e => { print(`write error: ${e}`) })
    stdout.setup(context.in, UV_TTY_MODE_RAW)
    stdout.bytes = 0
    stdout.name = 'recv.stdout'
    stdin.close() // closes server, stops any new connections
}

function onEnd(fd) {
    print(`onEnd`)
}

function onData(fd, len) {
    const { stdout } = contexts[fd]
    if (stdin.bytes === 0) {
        start(stdin)
        start(stdout)
    }
    stdin.bytes += len
    const r = stdout.write(len)
    if (r < 0) {
        print(`write error: ${r} - ${stdout.error(r)}`)
        return stdout.close()
    }
    stdout.bytes += len // assume they will go
    if (r < len && stdout.queueSize() >= MAX_BUFFER) stdin.pause(fd)
}

function onClose(fd) {
    stop(stdin)
    printStats(stdin, 'in', fd)
    const { stdout } = contexts[fd]
    stdout.close()
}

const stdin = new Socket(UNIX)

stdin.bytes = 0
stdin.name = 'recv.stdin'

stdin.onConnect(onConnect)
stdin.onData(onData)
stdin.onClose(onClose)
stdin.onEnd(onEnd)
stdin.onError(e => print(`read error: ${e}`))

const r = stdin.listen(args[2] || '/tmp/pipe.sock')
if(r !== 0) {
    print(`listen: ${r}, ${stdin.error(r)}`)
}
