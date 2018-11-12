const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})
const { SecureContext, SecureSocket } = module('openssl', {})

const createContext = () => {
    if (contexts.length) return contexts.shift()
    const work = createBuffer(16384)
    const parser = new HTTPParser()
    const context = { in: createBuffer(16384), out: createBuffer(16384), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), request: { major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', body: [] }, client: null }
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
    const { client, out } = context
    const len = out.write(r200, 0)
    const r = client.write(len)
    if (r === len) return
    if (r < len) return client.pause()
    if (r < 0) client.close()
}

const setSecure = (sock, context) => {
    const secureClient = new SecureSocket()
    secureClient.setup(defaultContext, sock)
    secureClient.onHost(hostname => {
        if (!hostname) return
        const context = secureContexts[hostname]
        if (!context) return false
        if (context.hostname === defaultContext.hostname) return
        return context
    })
    secureClient.onError((code, message) => {
        print(`SSL Error (${code}): ${message}`)
        sock.close()
    })
    sock.onClose(() => {
        contexts.push(context)
        secureClient.finish()
    })
    sock.write = len => secureClient.write(len)
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
    client.onDrain(() => client.resume())
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.setNoDelay(true)
    client.setKeepAlive(true, 3000)
    setSecure(client, context)
    parser.reset(REQUEST, client)
}

const createSecureContext = (hostname, cert, key) => {
    const secureContext = new SecureContext()
    secureContext.setup(cert, key)
    secureContext.hostname = hostname
    return secureContext
}

const secureContexts = {
    'dv8.billywhizz.io': createSecureContext('dv8.billywhizz.io', './dv8.billywhizz.io.cert.pem', './dv8.billywhizz.io.key.pem'),
    'foo.billywhizz.io': createSecureContext('foo.billywhizz.io', './foo.billywhizz.io.cert.pem', './foo.billywhizz.io.key.pem')
}
const defaultContext = secureContexts['dv8.billywhizz.io']

let r200 = `HTTP/1.1 200 OK\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 0\r\n\r\n`
const timer = setInterval(() => (r200 = `HTTP/1.1 200 OK\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 0\r\n\r\n`), 1000)
const contexts = []
const server = new Socket(TCP)
server.onConnect(onClient)
const r = server.listen('0.0.0.0', 3000)
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
