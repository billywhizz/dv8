const tty = module('tty', {})
const { TTY } = tty
const b = new Buffer()
b.alloc(64 * 1024)
stdin = new TTY(0, len => {
    const text = b.pull(0, len)
}, () => stdin.close(), () => stdout.close())
stdout = new TTY(1, () => {}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b, tty.UV_TTY_MODE_RAW)
stdout.setup(b)
stdin.resume()
