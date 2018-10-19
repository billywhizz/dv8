require('./lib/base.js')
const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { Socket, UNIX } = module('socket', {})
const { start, stop } = require('./lib/meter.js')

const sock = new Socket(UNIX)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE

sock.onConnect(fd => {
    const stdin = new TTY(0)
    const stdout = new Socket(UNIX)
    const buf = createBuffer(BUFFER_SIZE)
    stdout.setup(fd, buf, buf)
    stdin.setup(buf, UV_TTY_MODE_RAW)
    stdin.name = 'send.stdin'
    stdout.name = 'send.stdout'
    stdin.onRead(len => {
        const r = stdout.write(len)
        if (r < 0) return stdout.close()
        if (r < len && stdout.queueSize() >= MAX_BUFFER) stdin.pause()
    })
    stdin.onEnd(() => {
        stop(stdin)
        stdin.close()
    })
    stdin.onClose(() => stdout.close())
    stdin.onError(err => print(`stdin.onError: ${err}`))
    stdout.onDrain(() => stdin.resume())
    stdout.onClose(() => stop(stdout))
    stdout.onError((e, message) => print(`stdout.error: ${e.toString()}\n${message}`))
    stdin.resume()
    start(stdin)
    start(stdout)
})

sock.onClose(() => print('sock.close'))
sock.onError(e => print(`sock.error: ${e}`))

const r = sock.connect(args[2] || '/tmp/pipe.sock')
if(r !== 0) print(`sock.connect: ${r}, ${sock.error(r)}`)
