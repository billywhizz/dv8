const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { Socket, UNIX } = module('socket', {})
const { start, stop } = require('./lib/meter.js')

const sock = new Socket(UNIX)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE

sock.onConnect(fd => {
    const stdin = new Socket(UNIX)
    const stdout = new TTY(1)
    const buf = createBuffer(BUFFER_SIZE)
    stdin.setup(fd, buf, buf)
    stdout.setup(buf, UV_TTY_MODE_RAW)
    stdin.name = 'recv.stdin'
    stdout.name = 'recv.stdout'
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
    stdin.onError(err => print(`stdin.error: ${err}`))
    stdout.onDrain(() => stdin.resume())
    stdout.onClose(() => stop(stdout))
    stdout.onError((err, message) => print(`stdout.error: ${err}\n${message}`))
    //sock.close()
    start(stdin)
    start(stdout)
})

sock.onClose(() => print('sock.close'))
sock.onError(err => print(`sock.error: ${err}`))

const r = sock.listen(args[2] || '/tmp/pipe.sock')
if(r !== 0) print(`listen: ${r}, ${sock.error(r)}`)
