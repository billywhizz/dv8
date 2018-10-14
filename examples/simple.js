require('./base.js')

const { OS } = module('os', {})
const { Socket } = module('socket', {})
const { HTTPParser } = require('./http-parser.js')

const os = new OS()
const SIGTERM = 15
let sock
let closing = false

function terminateHandler(signum) {
    sock.close()
    closing = true
    return 1
}

os.onSignal(terminateHandler, SIGTERM)

const READ_BUFFER_SIZE = 4096
const WRITE_BUFFER_SIZE = 4096
const r200 = `HTTP/1.1 200 OK\r\nServer: ${threadId}\r\nContent-Length: 0\r\n\r\n`
const r200Close = `HTTP/1.1 200 OK\r\nConnection: close\r\nServer: ${threadId}\r\nContent-Length: 0\r\n\r\n`
const size200 = r200.length
const size200Close = r200Close.length

const contexts = {}
let r = 0

sock = new Socket(0)

function createContext(fd) {
    const context = {
        fd,
        in: new Buffer(),
        out: new Buffer()
    }
    context.in.ab = new Uint8Array(context.in.alloc(READ_BUFFER_SIZE))
    context.out.ab = new Uint8Array(context.out.alloc(WRITE_BUFFER_SIZE))
    contexts[fd] = context
    context.parser = new HTTPParser(HTTPParser.REQUEST)
    context.parser[HTTPParser.kOnMessageComplete] = () => {
        if (closing) {
            sock.write(fd, size200, size200Close)
            sock.close(fd, 1)
            return
        }
        sock.write(fd, 0, size200)
    }
    sock.setup(fd, context.in, context.out)
    return context
}

sock.onConnect(fd => {
    let context = contexts[fd]
    if (!context) {
        context = createContext(fd)
    } else {
        context.parser.reinitialize(HTTPParser.REQUEST)
    }
    sock.setNoDelay(fd, false)
    sock.setKeepAlive(fd, 1, 5)
    sock.push(fd, r200, 0)
    sock.push(fd, r200Close, size200)
})

sock.onData((fd, len) => {
    const context = contexts[fd]
    const { parser } = context
    parser.execute(context.in, 0, len)
})

sock.listen('0.0.0.0', 3000)
