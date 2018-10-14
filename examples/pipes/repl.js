/*
A very crappy repl so you can inspect some of the builtins
*/
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})
const b = new Buffer()
const ab = new Uint8Array(b.alloc(64 * 1024))

let payload
let written
let r

const stdin = new TTY(0, len => {
    const text = b.read(0, len)
    const res = eval(text)
    payload = `${JSON.stringify(res, null, 2)}\n`
    if (!res) payload = '(null)\n'
    if (!payload) payload = '(null)\n'
    if (payload == undefined) payload = '(undefined)\n'
    written = b.write(payload, 0)
    r = stdout.write(written)
    if (r < 0) return stdout.close()
    payload = '> '
    written = b.write(payload, 0)
    r = stdout.write(written)
    if (r < 0) return stdout.close()
}, () => stdin.close(), () => stdout.close())
const stdout = new TTY(1, () => {}, () => stdin.resume(), e => { print(`write error: ${e}`) })
print(`normal: ${UV_TTY_MODE_NORMAL}`)
print(`raw   : ${UV_TTY_MODE_RAW}`)
print(`io    : ${UV_TTY_MODE_IO}`)
stdin.setup(b, UV_TTY_MODE_NORMAL)
stdout.setup(b, UV_TTY_MODE_NORMAL)

payload = '> '
written = b.write(payload, 0)
r = stdout.write(written)
if (r < 0) {
    stdout.close()
} else {
    stdin.resume()
}

