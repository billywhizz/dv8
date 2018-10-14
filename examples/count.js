const { start, stop } = require('./meter.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})

function printStats(pipe, direction = 'in') {
    const stats = new BigUint64Array(20)
    pipe.stats(stats)
    print(`
COUNT.${direction}
-----------------------
close:     ${stats[0]}
error:     ${stats[1]}
read:      ${stats[2]}
pause:     ${stats[3]}
data:      ${stats[4]}
resume:    ${stats[5]}
end:       ${stats[6]}
`)
}

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

const stdin = new TTY(0, onRead, onEnd, onClose)
const b = new Buffer()
stdin.buffer = b.alloc(64 * 1024)
stdin.bytes = 0
stdin.name = 'count.stdin'
stdin.setup(b, UV_TTY_MODE_RAW)
stdin.resume()
