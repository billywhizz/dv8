const { TTY } = module('tty', {})
const b = new Buffer()
b.alloc(64 * 1024)
const stdin = new TTY(0, len => {
    const text = b.pull(0, len)
    const res = `${JSON.stringify(eval(text), null, '  ')}\n`
    b.push(res, 0)
    const r = stdout.write(res.length)
    if (r < 0) return stdout.close()
}, () => stdin.close(), () => stdout.close())
const stdout = new TTY(1, () => {}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b)
stdout.setup(b)
stdin.resume()
