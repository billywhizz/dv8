/*
A very crappy repl so you can inspect some of the builtins
*/
const tty = module('tty', {})
const { TTY } = tty
const b = new Buffer()
b.alloc(64 * 1024)
stdin = new TTY(0, len => {
    const text = b.read(0, len)
    const res = `${JSON.stringify(eval(text), null, '  ')}\n`
    b.write(res, 0)
    const r = stdout.write(res.length)
    if (r < 0) return stdout.close()
}, () => stdin.close(), () => stdout.close())
stdout = new TTY(1, () => {}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b)
stdout.setup(b)
stdin.resume()
