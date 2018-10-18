const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { Socket, UNIX } = module('socket', {})
const { start, stop } = require('./lib/meter.js')
const { printStats, createBuffer } = require('./lib/util.js')

const client = new Socket(UNIX)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE

client.onConnect(fd => {
    const stdin = new TTY(0)
    const stdout = new Socket(UNIX)
    const buf = createBuffer(BUFFER_SIZE)
    stdout.setup(fd, buf, buf)
    stdin.setup(buf, UV_TTY_MODE_RAW)
    stdin.bytes = 0
    stdin.name = 'send.stdin'
    stdout.bytes = 0
    stdout.name = 'send.stdout'
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
    stdin.resume()
})

client.onClose(() => {
    print('client.close')
})

client.onError(e => print(`client.error: ${e}`))

const r = client.connect(args[2] || '/tmp/pipe.sock')
if(r !== 0) {
    print(`connect: ${r}, ${client.error(r)}`)
}
