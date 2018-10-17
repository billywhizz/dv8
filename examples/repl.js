/*
A very crappy repl so you can inspect some of the builtins
*/
const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})
const b = new Buffer()
const ab = new Uint8Array(b.alloc(64 * 1024))

let payload
let written
let r

const MAXBUF = 4 * 64 * 1024
const stdin = new TTY(0)
stdin.setup(b, UV_TTY_MODE_NORMAL)

stdin.onRead(len => {
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
    if (r < len && stdout.queueSize() >= MAXBUF) stdin.pause()
})

stdin.onEnd(() => {
    stdin.close()
})

stdin.onClose(() => {
    stdout.close()
})

const stdout = new TTY(1, () => {}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdout.setup(b, UV_TTY_MODE_NORMAL)

stdout.onClose(() => {

})

stdout.onDrain(() => {
    stdin.resume()
})

stdout.onError((e, message) => {
    print(`stdout.error:\n${e.toString()}\n${message}`)
})


payload = '> '
written = b.write(payload, 0)
r = stdout.write(written)
if (r < 0) {
    stdout.close()
} else {
    stdin.resume()
}
