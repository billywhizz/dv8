const tty = module('tty', {})
const { TTY } = tty
const { Timer } = module('timer', {})
const b = new Buffer()
b.alloc(64 * 1024)
let start = Date.now()
function displayRate() {
    const finish = Date.now()
    const elapsed = finish - start
    const elapsedSeconds = BigInt(Math.ceil(elapsed / 1000))
    const stin = new BigUint64Array(7)
    stdin.stats(stin)
    const readRate = (stin[0] / elapsedSeconds) * 8n / (1024n * 1024n)
    const stout = new BigUint64Array(10)
    stdout.stats(stout)
    const writeRate = (stout[0] / elapsedSeconds) * 8n / (1024n * 1024n)
    print(`read: ${readRate}, write: ${writeRate}`)
}
const stdin = new TTY(0, len => {
    const r = stdout.write(len)
    if (r < 0) return stdout.close()
    if (r < len && stdout.queueSize() >= (64 * 1024)) stdin.pause()
}, () => stdin.close(), () => stdout.close())
const stdout = new TTY(1, () => {
    timer.stop()
    displayRate()
}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b)
stdout.setup(b)
stdin.resume()
const timer = new Timer()
timer.start(() => displayRate(), 1000, 1000)
