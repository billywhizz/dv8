const { Socket, TCP } = module('socket', {})
const { setSecure, addContext } = require('./lib/tls.js')

const [ rb, wb ] = [ createBuffer(16384), createBuffer(16384) ]
const sock = new Socket(TCP)

function writeText(sock, str) {
    const len = str.length
    wb.write(str, 0)
    const r = sock.write(len)
    if (r < 0) return sock.close()
}

sock.onConnect(fd => {
    sock.setup(fd, rb, wb)
    sock.onRead(len => print(rb.read(0, len)))
    sock.onEnd(() => sock.close())
    sock.setNoDelay(true)
    sock.setKeepAlive(true, 3000)
    sock.address = sock.remoteAddress()
    setSecure(sock, () => writeText(sock, `GET / HTTP/1.1\r\nHost: foo\r\n\r\n`))
    sock.start()
})

addContext('127.0.0.1', false)
const r = sock.connect('127.0.0.1', 3000)
if (r !== 0) throw new Error(`Connect Error: ${r} ${sock.error(r)}`)
