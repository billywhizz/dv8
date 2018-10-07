const { start, stop } = require('./meter.js')
const { TTY } = module('tty', {})
const b = new Buffer()
b.alloc(64 * 1024)
const stdin = new TTY(0, len => stdin.bytes += len, () => {
    stdin.close()
    stop(stdin)
}, () => {})
stdin.setup(b)
stdin.bytes = 0
stdin.name = 'count.stdin'
stdin.timer = start(stdin)
stdin.resume()
