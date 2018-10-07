const { start, stop } = require('./meter.js')
const tty = module('tty', {})
const { TTY } = tty
const b = new Buffer()
b.alloc(64 * 1024)
const stdin = new TTY(0, len => {
    stdin.bytes += len
    const r = stdout.write(len)
    if (r < 0) return stdout.close()
    stdout.bytes += len // assume they will go
    if (r < len && stdout.queueSize() >= (64 * 1024)) stdin.pause()
}, () => {
    stop(stdin)
    stdin.close()
}, () => stdout.close())
const stdout = new TTY(1, () => {
    stop(stdout)
}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b)
stdout.setup(b)
stdin.bytes = 0
stdout.bytes = 0
stdin.name = 'pipe.stdin'
stdout.name = 'pipe.stdout'
start(stdin)
start(stdout)
stdin.resume()