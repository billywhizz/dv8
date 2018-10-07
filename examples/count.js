/*
Count the bytes piped to stdin
*/
// import the meter module for recording times and displaying them
const { start, stop } = require('./meter.js')
// import the tty binary module
const { TTY } = module('tty', {})
// create a new buffer
const b = new Buffer()
// allocate 64k outside the v8 heap
b.alloc(64 * 1024)
// open stdin, with event handlers for onRead, onEnd, onClose 
const stdin = new TTY(0, len => stdin.bytes += len, () => {
    // close stdin TTY
    stdin.close()
    // stop the metrics
    stop(stdin)
}, () => {})
// hand the buffer to the stdin TTY
stdin.setup(b)
// bytes and name used in metrics module
stdin.bytes = 0
stdin.name = 'count.stdin'
// start the metrics and get a timer back
start(stdin)
// start reading stdin, paused initially
stdin.resume()
