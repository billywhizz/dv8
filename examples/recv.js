const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { Socket, UNIX } = module('socket', {})
const { start, stop } = require('./lib/meter.js')
const { printStats, createBuffer } = require('./lib/util.js')

const server = new Socket(UNIX)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE

server.onConnect(fd => {
    const stdin = new Socket(UNIX)
    const stdout = new TTY(1)
    const buf = createBuffer(BUFFER_SIZE)
    stdin.setup(fd, buf, buf)
    stdout.setup(buf, UV_TTY_MODE_RAW)
    stdin.bytes = 0
    stdin.name = 'recv.stdin'
    stdout.bytes = 0
    stdout.name = 'recv.stdout'
    stdin.onRead(len => {
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
        if (r < len && stdout.queueSize() >= MAX_BUFFER) stdin.pause()
    })
    stdin.onClose(() => {
        printStats(stdin)
        stdout.close()
    })
    stdin.onError(err => {
        print(`server.onError: ${err}`)
    })
    stdin.onEnd(() => {
        stop(stdin)
        print(`server.onEnd`)
        stdin.close()
    })
    stdout.onClose(() => {
        stop(stdout)
        printStats(stdout, 'out')
    })
    stdout.onDrain(() => {
        stdin.resume()
    })
    stdout.onError((e, message) => {
        print(`stdout.error: ${e.toString()}\n${message}`)
    })
    server.close() // closes server, stops any new connections
})

server.onClose(() => {
    print('server.close')
})

server.onError(e => print(`server.error: ${e}`))

const r = server.listen(args[2] || '/tmp/pipe.sock')
if(r !== 0) {
    print(`listen: ${r}, ${server.error(r)}`)
}
