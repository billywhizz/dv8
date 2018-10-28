const { Socket, UNIX } = module('socket', {})
const { start, stop } = require('./lib/meter.js')

const ITERATIONS = parseInt(args[2] || 1, 10)
const BUFFER_SIZE = parseInt(args[3] || 64 * 1024, 10)

let remaining = BUFFER_SIZE * ITERATIONS

const stdout = new Socket(UNIX)
let timer

function writeAll() {
    clearTimeout(timer)
    if (remaining === 0) return stdout.close()
    let len = 0
    let r = 0
    while (r === len) {
        len = Math.min(BUFFER_SIZE, remaining)
        r = stdout.write(len)
        if (r < 0) return stdout.close()
        if (remaining === 0) return stdout.close()
    }
}

stdout.onConnect(fd => {
    const buf = createBuffer(BUFFER_SIZE)
    stdout.setup(fd, buf, buf)
    stdout.name = 'dd.stdout'
    stdout.onClose(() => stop(stdout))
    stdout.onError((err, message) => print(`stdout.error: ${err}\n${message}`))
    stdout.onWrite(len => {
        remaining -= len
    })
    stdout.onDrain(() => timer = setTimeout(writeAll, 0))
    start(stdout)
    timer = setTimeout(writeAll, 1)
})
stdout.onClose(() => print('sock.close'))
stdout.onError(e => print(`sock.error: ${e}`))

const r = stdout.connect(args[4] || '/tmp/pipe.sock')
if(r !== 0) print(`sock.connect: ${r}, ${stdout.error(r)}`)
