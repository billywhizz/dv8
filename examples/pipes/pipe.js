const { start, stop } = require('./meter.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})
const { printStats } = require('./util.js')
const b = new Buffer()
b.bytes = new Uint8Array(b.alloc(64 * 1024))
function handler(buf, len, stdin, stdout) {
    const r = stdout.write(len)
    if (r < 0) {
        stdout.close()
        return 0
    }
    if (r < len && stdout.queueSize() >= (64 * 1024)) stdin.pause()
    return len
}
const stdin = new TTY(0, len => {
    if (stdin.bytes === 0) {
        start(stdin)
        start(stdout)
    }
    stdin.bytes += len
    stdout.bytes += handler(b, len, stdin, stdout)
}, () => {
    stop(stdin)
    stdin.close()
}, () => {
    printStats(stdin)
    stdout.close()
})
const stdout = new TTY(1, () => {
    stop(stdout)
    printStats(stdout, 'out')
}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b, UV_TTY_MODE_RAW)
stdout.setup(b, UV_TTY_MODE_RAW)
stdin.bytes = 0
stdout.bytes = 0
stdin.name = 'pipe.stdin'
stdout.name = 'pipe.stdout'
stdin.resume()