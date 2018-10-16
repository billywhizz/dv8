const { start, stop } = require('./lib/meter.js')
const { printStats } = require('./lib/util.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})

function onRead(len) {
    if (stdin.bytes === 0) start(stdin)
    stdin.bytes += len
}

function onEnd() {
    stdin.close()
    stop(stdin)
}

function onClose() {
    printStats(stdin)
}

const stdin = new TTY(0)
const b = new Buffer()
b.alloc(64 * 1024)
stdin.bytes = 0
stdin.name = 'count.stdin'
stdin.setup(b, UV_TTY_MODE_RAW)
stdin.onRead(onRead)
stdin.onEnd(onEnd)
stdin.onClose(onClose)
stdin.resume()
