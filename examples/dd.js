const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { start, stop } = require('./lib/meter.js')

const ITERATIONS = parseInt(args[2] || 1, 10)
const BUFFER_SIZE = parseInt(args[3] || 64 * 1024, 10)

let remaining = BUFFER_SIZE * ITERATIONS

const stdout = new TTY(1)
let timer

function writeAll() {
    clearTimeout(timer)
    if (remaining === 0) return stdout.close()
    let len = 0
    let r = 0
    while (r === len) {
        len = Math.min(BUFFER_SIZE, remaining)
        r = stdout.write(len)
        if (r < 0) return stdout.close()
        if (remaining === 0) return stdout.close()
    }
}

const buf = createBuffer(BUFFER_SIZE)
stdout.setup(buf, UV_TTY_MODE_RAW)
stdout.name = 'dd.stdout'
stdout.onClose(() => stop(stdout))
stdout.onError((err, message) => print(`stdout.error: ${err}\n${message}`))
stdout.onWrite(len => {
    remaining -= len
})
stdout.onDrain(() => timer = setTimeout(writeAll, 0))
start(stdout)
timer = setTimeout(writeAll, 1)
