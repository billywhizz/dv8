/*
Count the bytes piped to stdin
*/
// import the meter module for recording times and displaying them
const { start, stop } = require('./meter.js')
const stuff = []
// import the tty binary module
const { TTY } = module('tty', {})
// create a new buffer
const b = new Buffer()
// allocate 64k outside the v8 heap
// open stdin, with event handlers for onRead, onEnd, onClose 
const stdin = new TTY(0, len => {
    if (stdin.bytes === 0) start(stdin)
    stdin.bytes += len
}, () => {
    // close stdin TTY
    stdin.close()
    // stop the metrics
    stop(stdin)
}, () => {
    const stats = new BigUint64Array(20)
    stdin.stats(stats)
    print(`
COUNT.stdin
-----------------------
close:     ${stats[0]}
error:     ${stats[1]}
read:      ${stats[2]}
pause:     ${stats[3]}
data:      ${stats[4]}
resume:    ${stats[5]}
end:       ${stats[6]}
`)
})
// hand the buffer to the stdin TTY
stdin.buffer = b.alloc(64 * 1024)
stdin.setup(b)
// bytes and name used in metrics module
stdin.bytes = 0
stdin.name = 'count.stdin'
// start reading stdin, paused initially
stdin.resume()
