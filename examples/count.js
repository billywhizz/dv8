require('./lib/base.js')
const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { start, stop } = require('./lib/meter.js')

const stdin = new TTY(0)
const BUFFER_SIZE = 64 * 1024

const buf = createBuffer(BUFFER_SIZE)
stdin.name = 'count.stdin'
stdin.setup(buf, UV_TTY_MODE_RAW)
stdin.onRead(len => {})
stdin.onEnd(() => stdin.close())
stdin.onClose(() => stop(stdin))
stdin.resume()
start(stdin)
