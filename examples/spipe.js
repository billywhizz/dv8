const { start, stop } = require('./meter.js')
const { Socket, TCP, UNIX } = module('socket', {})
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
        const stats = new BigUint64Array(20)
        stdin.stats(stats)
        print(`
NETCAT.stdin
-----------------------
close:     ${stats[0]}
error:     ${stats[1]}
read:      ${stats[2]}
pause:     ${stats[3]}
data:      ${stats[4]}
resume:    ${stats[5]}
end:       ${stats[6]}
`)
        stdout.close(fd)
    })
    
    stdin.setup(b, UV_TTY_MODE_RAW)
    
    stdin.bytes = 0
    stdout.bytes = 0
    stdin.name = 'pipe.stdin'
    stdout.name = 'pipe.socket'

    stdout.onDrain(fd => {
        stdin.resume()
    })

    stdin.resume()
})

stdout.onClose(fd => {
    stop(stdout)
    const stats = new BigUint64Array(20)
    stdout.stats(fd, stats)
    print(`
NETCAT.stdout
-----------------------
close:      ${stats[0]}
error:      ${stats[1]}
written:    ${stats[10]}
incomplete: ${stats[11]}
full:       ${stats[12]}
drain:      ${stats[13]}
maxQueue:   ${stats[14]}
alloc:      ${stats[15]}
free:       ${stats[16]}
eagain:     ${stats[17]}
`)
})

stdout.onError((fd, e) => print(`write error: ${e}`))

stdout.connect('/tmp/pipe.sock')
