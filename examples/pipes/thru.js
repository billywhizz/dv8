const { start, stop } = require('./meter.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})
const { printStats } = require('./util.js')
const b = new Buffer()
const bytes = new Uint8Array(b.alloc(64 * 1024))

module.exports = {
    UV_TTY_MODE_RAW,
    UV_TTY_MODE_NORMAL,
    UV_TTY_MODE_IO,
    pipe: (handler, inmode = UV_TTY_MODE_RAW, outmode = UV_TTY_MODE_RAW, name = 'thru') => {
        const stdin = new TTY(0, len => {
            if (stdin.bytes === 0) {
                start(stdin)
                start(stdout)
            }
            stdin.bytes += len
            stdout.bytes += handler(b, len, stdin, stdout)
        }, () => {
            stop(stdin)
            stdin.close()
        }, () => {
            printStats(stdin)
            stdout.close()
        })
        const stdout = new TTY(1, () => {
            stop(stdout)
            printStats(stdout, 'out')
        }, () => stdin.resume(), e => { print(`write error: ${e}`) })
        stdin.setup(b, inmode)
        stdout.setup(b, outmode)
        stdin.bytes = 0
        stdout.bytes = 0
        stdin.name = `${name}.stdin`
        stdout.name = `${name}.stdout`
        stdin.resume()
    }
}
