const { UV_TTY_MODE_NORMAL, TTY } = module('tty', {})
const { start, stop } = require('./meter.js')
const stdin = new TTY(0)
const BUFFER_SIZE = 256 * 1024
const buf = Buffer.alloc(BUFFER_SIZE)
stdin.name = 'count.stdin'
stdin.setup(buf, UV_TTY_MODE_NORMAL)
stdin.onEnd(() => stdin.close())
stdin.onClose(() => stop(stdin))
stdin.resume()
start(stdin)
