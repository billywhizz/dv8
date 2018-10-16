const { start, stop } = require('./lib/meter.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO, UV_EAGAIN, UV_EOF, TTY } = module('tty', {})
const { printStats } = require('./lib/util.js')

const MAXBUF = 4 * 64 * 1024
const buf = new Buffer()
buf.bytes = new Uint8Array(buf.alloc(64 * 1024))

const stdin = new TTY(0)
stdin.setup(buf, UV_TTY_MODE_RAW)
stdin.bytes = 0
stdin.name = 'pipe.stdin'

stdin.onRead(len => {
    if (stdin.bytes === 0) {
        start(stdin)
        start(stdout)
    }
    stdin.bytes += len
    const r = stdout.write(len)
    if (r < 0) {
        stdout.close()
        return
    }
    if (r < len && stdout.queueSize() >= MAXBUF) stdin.pause()
    stdout.bytes += len
})

stdin.onEnd(() => {
    stop(stdin)
    stdin.close()
})

stdin.onClose(() => {
    printStats(stdin)
    stdout.close()
})

stdin.onError(e => {
    print(`stdin.error: ${e.toString()}`)
})

const stdout = new TTY(1)
stdout.setup(buf, UV_TTY_MODE_RAW)
stdout.bytes = 0
stdout.name = 'pipe.stdout'

stdout.onClose(() => {
    stop(stdout)
    printStats(stdout, 'out')
})

stdout.onDrain(() => {
    stdin.resume()    
})
/*
stdout.onWrite(len => {

})

stdout.onEnd(() => {

})
*/
stdout.onError((e, message) => {
    print(`stdout.error: ${e.toString()}\n${message}`)
})

stdin.resume()