require('./lib/base.js')

const { OS } = module('os', {})
const { Socket, TCP, UNIX } = module('socket', {})
const { HTTPParser } = require('./lib/http-parser.js')

const os = new OS()
const SIGTERM = 15
let timer
let sock
let id = 0
let closing = false
let conn = 0
let bytesRead = 0
let bytesWritten = 0

onExit(() => {
    printMetrics()
}) 

function terminateHandler(signum) {
    print(`worker ${id} got SIGNAL: ${signum}`)
    sock.close()
    closing = true
    clearTimeout(timer)
    return 1
}

function printMetrics() {
    print(JSON.stringify({
        worker: id,
        connections: conn,
        bytesRead,
        bytesWritten
    }, null, '  '))
}

if (global.workerData) {
    const data = new Uint8Array(workerData)
    id = data[0]
}

os.onSignal(terminateHandler, SIGTERM)

const BUFFER_SIZE = 4096
const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const r200Close = 'HTTP/1.1 200 OK\r\nConnection: close\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const size200 = r200.length
const size200Close = r200Close.length

let r = 0

sock = new Socket(TCP)

function createBuffer(size) {
    const buf = new Buffer()
    buf.bytes = new Uint8Array(buf.alloc(size))
    return buf
}

sock.onConnect(fd => {
    conn++
    const client = new Socket(TCP)
    client.fd = fd
    client.in = createBuffer(BUFFER_SIZE)
    client.out = createBuffer(BUFFER_SIZE)
    client.setup(fd, client.in, client.out)
    const parser = new HTTPParser(HTTPParser.REQUEST)
    parser[HTTPParser.kOnMessageComplete] = () => {
        if (closing) {
            bytesWritten += client.write(size200, size200Close)
            client.close(1)
            return
        }
        client.write(0, size200)
    }
    client.onRead(len => {
        bytesRead += len
        parser.execute(client.in, 0, len)
    })
    client.onClose(fd => {
        conn--
    })
    client.onWrite((len, status) => {

    })
    client.onDrain(() => {
        print(`server.onDrain`)
    })
    client.onError(err => {
        print(`server.onError: ${err}`)
    })
    client.onEnd(() => {
        print(`server.onEnd`)
    })
    client.setNoDelay(false)
    client.setKeepAlive(1, 5)
    client.out.write(r200, 0)
    client.out.write(r200Close, size200)
})

//timer = setInterval(printMetrics, 1000)

r = sock.listen('0.0.0.0', 3000)
if (r !== 0) {
    throw new Error("Could not Listen")
}
