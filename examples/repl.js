const { UV_TTY_MODE_NORMAL, TTY } = module('tty', {})

const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE
const stdin = new TTY(0)
const buf = createBuffer(BUFFER_SIZE)

function parse(obj){
    return Function('"use strict";return (' + obj + ')')();
}

stdin.setup(buf, UV_TTY_MODE_NORMAL)
stdin.onRead(len => {
    const source = buf.read(0, len)
    const result = parse(source)
    let payload = `${JSON.stringify(result, null, 2)}\n`
    if (!result) payload = '(null)\n'
    if (!payload) payload = '(null)\n'
    if (payload == undefined) payload = '(undefined)\n'
    let r = stdout.write(buf.write(payload, 0))
    if (r < 0) return stdout.close()
    r = stdout.write(buf.write('> ', 0))
    if (r < 0) return stdout.close()
    if (r < len && stdout.queueSize() >= MAX_BUFFER) stdin.pause()
})
stdin.onEnd(() => stdin.close())
stdin.onClose(() => stdout.close())

const stdout = new TTY(1)
stdout.setup(buf, UV_TTY_MODE_NORMAL)
stdout.onDrain(() => stdin.resume())
stdout.onError((e, message) => print(`stdout.error:\n${e.toString()}\n${message}`))
if (stdout.write(buf.write('> ', 0)) < 0) {
    stdout.close()
} else {
    stdin.resume()
}
