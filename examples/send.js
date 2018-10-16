const { start, stop } = require('./lib/meter.js')
const { Socket, TCP, UNIX } = module('socket', {})
const { printStats } = require('./lib/util.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})

const READ_BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * READ_BUFFER_SIZE

const b = new Buffer()
b.alloc(READ_BUFFER_SIZE)

const stdout = new Socket(UNIX)

stdout.onConnect(fd => {
    stdout.setup(fd, b, b)
    const stdin = new TTY(0, len => {
        if (stdin.bytes === 0) {
            start(stdin)
            start(stdout)
        }
        stdin.bytes += len
        const r = stdout.write(fd, 0, len)
        if (r < 0) {
            return stdout.close(fd, 0)
        }
        stdout.bytes += len // assume they will go
        if (r < len && stdout.queueSize(fd) >= MAX_BUFFER) {
            stdin.pause()
        }
    }, () => {
        stdin.close()
        stop(stdin)
    }, () => {
        printStats(stdin, 'in')
        stdout.close(fd)
    })
    
    stdin.setup(b, UV_TTY_MODE_RAW)
    
    stdin.bytes = 0
    stdout.bytes = 0
    stdin.name = 'send.stdin'
    stdout.name = 'send.stdout'

    stdout.onDrain(fd => {
        stdin.resume()
    })

    stdin.resume()
})

stdout.onClose(fd => {
    stop(stdout)
    printStats(stdout, 'out', fd)
})

stdout.onError((fd, e) => print(`write error: ${e}`))

const r = stdout.connect(args[2] || '/tmp/pipe.sock')
if(r !== 0) {
    print(`connect: ${r}, ${stdout.error(r)}`)
}
