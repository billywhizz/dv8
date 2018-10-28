const { OS } = module('os', {})
const { Socket, TCP } = module('socket', {})
const { HTTPParser } = require('./lib/http-parser.js')

const sock = new Socket(TCP)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE
const { HTTPD_LISTEN_ADDRESS, HTTPD_LISTEN_PORT } = env

const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const r200Close = 'HTTP/1.1 200 OK\r\nConnection: close\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const SIGTERM = 15
const os = new OS()

const { kOnMessageComplete } = HTTPParser

let closing = false
let timer

const contexts = []

function createContext() {
    if (!contexts.length) {
        const context = { in: createBuffer(BUFFER_SIZE), out: createBuffer(BUFFER_SIZE) }
        context.in.bytes = new Uint8Array(context.in.bytes)
        return context
    }
    return contexts.shift()
}

function freeContext(context) {
    contexts.push(context)
}

onExit(() => printMetrics())

function terminateHandler(signum) {
    sock.close()
    closing = true
    clearTimeout(timer)
    return 1
}

os.onSignal(terminateHandler, SIGTERM)

sock.onConnect(fd => {
    const client = new Socket(TCP)
    const parser = new HTTPParser(HTTPParser.REQUEST)
    const context = createContext()
    context.out.write(r200, 0)
    context.out.write(r200Close, r200.length)
    client.setup(fd, context.in, context.out)
    parser[kOnMessageComplete] = () => {
        if (closing) {
            client.write(r200Close.length, r200.length)
            client.close()
            return
        }
        const r = client.write(r200.length)
        if (r < 0) return client.close()
        if (r < r200.length && client.queueSize() >= MAX_BUFFER) client.pause()
    }
    client.onRead(len => parser.execute(context.in, 0, len))
    client.onDrain(() => client.resume())
    client.onClose(() => freeContext(context))
    client.onError(err => print(`client.error: ${err} ${client.error(err)}`))
    client.onEnd(() => client.close())
    client.setNoDelay(false)
    client.setKeepAlive(1, 5)
})

const r = sock.listen(HTTPD_LISTEN_ADDRESS || '0.0.0.0', parseInt(HTTPD_LISTEN_PORT || args[2] || 3000, 10))
if (r !== 0) throw new Error(`Listen Error: ${r} ${sock.error(r)}`)
