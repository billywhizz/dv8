const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const { HTTPD_LISTEN_ADDRESS, HTTPD_LISTEN_PORT } = env
const IN_BUFFER_SIZE = 64 * 1024
const OUT_BUFFER_SIZE = 4 * 1024
const WORK_BUFFER_SIZE = 4 * 1024
const STRING_START = 16
const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const r200len = r200.length
const contexts = []
const buffers = { in: createBuffer(IN_BUFFER_SIZE), out: createBuffer(OUT_BUFFER_SIZE) }

function createContext() {
    if (contexts.length) return contexts.shift()
    const work = createBuffer(WORK_BUFFER_SIZE)
    const request = { address: '127.0.0.1', major: 0, minor: 0, method: 0, upgrade: 0, keepalive: 0, headers: '', url: '' }
    const parser = new HTTPParser()
    const bytes = new Uint8Array(work.bytes)
    const view = new DataView(work.bytes)
    const context = { in: buffers.in, out: buffers.out, work, parser, bytes, view, request }
    context.parser.setup(context.in, work)
    return context
}

function splitHeader(header) {
    const first = header.indexOf(':')
    return [header.substring(0, first).trim(), header.substring(first + 1).trim()]
}

function parseHeaders(context) {
    const { client, bytes, view, work, request } = context
    request.address = client.address
    request.major = bytes[0]
    request.minor = bytes[1]
    request.method = bytes[2]
    request.upgrade = bytes[3]
    request.keepalive = bytes[4]
    const urlLength = view.getUint16(5)
    const headerLength = view.getUint16(7)
    const str = work.read(STRING_START, STRING_START + urlLength + headerLength)
    request.url = str.substring(0, urlLength)
    request.headers = str.substring(urlLength, urlLength + headerLength)
    //request.headers = str.substring(urlLength, urlLength + headerLength).split('\r\n').map(splitHeader)
}

function onRequest(context) {
    const { client } = context
    const r = client.write(r200len)
    if (r === r200len) return
    if (r < r200len) return client.pause()
    if (r < 0) client.close()
}

function onClient(fd) {
    const client = new Socket(TCP)
    const context = createContext()
    const { work, parser } = context
    const body = []
    context.client = client
    parser.onBody(len => body.push(work.read(0, len)))
    parser.onHeaders(() => parseHeaders(context))
    parser.onRequest(() => onRequest(context))
    parser.reset(REQUEST, client)
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onDrain(() => client.resume())
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.setNoDelay(false)
}

const server = new Socket(TCP)
buffers.out.write(r200, 0)
server.onConnect(onClient)
const r = server.listen(HTTPD_LISTEN_ADDRESS || '0.0.0.0', parseInt(HTTPD_LISTEN_PORT || 3000, 10))
if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)
