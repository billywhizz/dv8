const { start, stop } = require('./meter.js')
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})
const b = new Buffer()
const bytes = new Uint8Array(b.alloc(64 * 1024))
const stdin = new TTY(0, len => {
    if (stdin.bytes === 0) {
        start(stdin)
        start(stdout)
    }
    stdin.bytes += len
    const r = stdout.write(len)
    if (r < 0) return stdout.close()
    stdout.bytes += len // assume they will go
    if (r < len && stdout.queueSize() >= (64 * 1024)) stdin.pause()
}, () => {
    stop(stdin)
    stdin.close()
}, () => {
    const stats = new BigUint64Array(20)
    stdin.stats(stats)
    print(`
PIPE.stdin
-----------------------
close:     ${stats[0]}
error:     ${stats[1]}
read:      ${stats[2]}
pause:     ${stats[3]}
data:      ${stats[4]}
resume:    ${stats[5]}
end:       ${stats[6]}
`)
    stdout.close()
})
const stdout = new TTY(1, () => {
    stop(stdout)
    const stats = new BigUint64Array(20)
    stdout.stats(stats)
    print(`
PIPE.stdout
-----------------------
close:      ${stats[0]}
error:      ${stats[1]}
written:    ${stats[10]}
incomplete: ${stats[11]}
full:       ${stats[12]}
drain:      ${stats[13]}
maxQueue:   ${stats[14]}
alloc:      ${stats[15]}
free:       ${stats[16]}
eagain:     ${stats[17]}
`)
}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b, UV_TTY_MODE_RAW)
stdout.setup(b, UV_TTY_MODE_RAW)
stdin.bytes = 0
stdout.bytes = 0
stdin.name = 'pipe.stdin'
stdout.name = 'pipe.stdout'
stdin.resume()