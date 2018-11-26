const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})
const { setSecure, addContext } = require('./lib/tls.js')

const createContext = () => {
    if (contexts.length) return contexts.shift()
    const work = Buffer.alloc(16384)
    const parser = new HTTPParser()
    const context = { in: Buffer.alloc(16384), out: Buffer.alloc(16384), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), request: { major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', body: [] }, client: null }
    parser.setup(context.in, work)
    return context
}

const onHeaders = context => {
    const { client, bytes, view, work, request } = context
    if (request.body.length) request.body.length = 0
    const urlLength = view.getUint16(5)
    const headerLength = view.getUint16(7)
    const str = work.read(16, 16 + urlLength + headerLength)
    request.major = bytes[0]
    request.minor = bytes[1]
    request.method = bytes[2]
    request.upgrade = bytes[3]
    request.keepalive = bytes[4]
    request.url = str.substring(0, urlLength)
    request.headers = str.substring(urlLength, urlLength + headerLength)
}

const onRequest = context => {
    const { client, out, request } = context
    const len = out.write(r200, 0)
    const r = client.write(len)
    if (r === len) {
        if (request.keepalive === 1) return
        if (client.queueSize() === 0) {
            return client.close()
        }
        client.onDrain(() => {
            client.close()
        })
        return
    }
    if (r < 0) client.close()
    if (r < len) {
        if (request.keepalive === 1) return client.pause()
        if (client.queueSize() === 0) {
            return client.close()
        }
        client.onDrain(() => client.close())
        return
    }
}

const onClient = fd => {
    const client = new Socket(TCP)
    const context = createContext()
    const { work, parser, request } = context
    const { body } = request
    context.client = client
    parser.onBody(len => body.push(work.read(0, len)))
    parser.onHeaders(() => onHeaders(context))
    parser.onRequest(() => onRequest(context))
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.setNoDelay(true)
    client.setKeepAlive(true, 3000)
    setSecure(client)
    parser.reset(REQUEST, client)
}

addContext('dv8.billywhizz.io')
addContext('foo.billywhizz.io')

let r200 = `HTTP/1.1 200 OK\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 0\r\n\r\n`
const timer = setInterval(() => (r200 = `HTTP/1.1 200 OK\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 0\r\n\r\n`), 1000)
const contexts = []
const server = new Socket(TCP)
server.onConnect(onClient)
const r = server.listen('0.0.0.0', 3000)
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
