require('./lib/base.js')
const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { start, stop } = require('./lib/meter.js')

const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE

const stdin = new TTY(0)
const stdout = new TTY(1)
const buf = createBuffer(BUFFER_SIZE)
stdin.setup(buf, UV_TTY_MODE_RAW)
stdout.setup(buf, UV_TTY_MODE_RAW)
stdin.name = 'pipe.stdin'
stdout.name = 'pipe.stdout'
stdin.onRead(len => {
    const r = stdout.write(len)
    if (r < 0) return stdout.close()
    if (r < len && stdout.queueSize() >= MAX_BUFFER) stdin.pause()
})
stdin.onEnd(() => {
    stop(stdin)
    stdin.close()
})
stdin.onClose(() => stdout.close())
stdin.onError(err => print(`stdin.error: ${err}`))
stdout.onDrain(() => stdin.resume())
stdout.onClose(() => stop(stdout))
stdout.onError((err, message) => print(`stdout.error: ${err}\n${message}`))
stdin.resume()
start(stdin)
start(stdout)
