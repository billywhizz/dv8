const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const { HTTPD_LISTEN_ADDRESS, HTTPD_LISTEN_PORT } = env
const IN_BUFFER_SIZE = 64 * 1024
const OUT_BUFFER_SIZE = 4 * 1024
const WORK_BUFFER_SIZE = 4 * 1024
const maxHeaders = 20
const HEADER_START = 12
const STRING_START = 128
const IDLE_CONTEXTS = 5000
const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const r200len = r200.length
const stats = { contexts: { free: 0, active: 0 } }
const contexts = []
const buffers = {
    in: createBuffer(IN_BUFFER_SIZE),
    out: createBuffer(OUT_BUFFER_SIZE)
}

function createContext() {
    if (contexts.length) {
        stats.contexts.active++
        stats.contexts.free--
        return contexts.shift()
    }
    const work = createBuffer(WORK_BUFFER_SIZE)
    const { bytes } = work
    const context = {
        in: buffers.in,
        out: buffers.out,
        work,
        parser: new HTTPParser(),
        bytes: new Uint8Array(bytes),
        view: new DataView(bytes),
        request: {
            address: '127.0.0.1',
            major: 0,
            minor: 0,
            method: 0,
            upgrade: 0,
            keepalive: 0,
            headers: new Array(maxHeaders),
            body: [],
            url: ''
        }
    }
    context.request.headers.fill('')
    context.parser.setup(context.in, context.work)
    stats.contexts.active++
    return context
}

function freeContext(context) {
    stats.contexts.active--
    if (contexts.length < IDLE_CONTEXTS) {
        stats.contexts.free++
        contexts.push(context)
    }
}

function parseHeaders(context) {
    const { client, bytes, view, work, request } = context
    request.address = client.address
    request.body.length = 0
    request.major = bytes[0]
    request.minor = bytes[1]
    request.method = bytes[2]
    request.upgrade = bytes[3]
    request.keepalive = bytes[4]
    let headerCount = bytes[5]
    const urlLength = view.getUint32(6)
    const stringLength = view.getUint16(10)
    let len = 0
    let curr = 0
    let soff = 0
    let off = HEADER_START
    const { headers } = request
    const str = work.read(STRING_START, STRING_START + stringLength)
    request.url = str.substring(soff, soff + urlLength)
    soff += urlLength
    while (headerCount--) {
        len = view.getUint16(off)
        headers[curr++] = str.substring(soff, soff + len)
        off += 2
        soff += len
    }
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
    const { work, request, parser } = context
    context.client = client
    parser.onBody(len => request.body.push(work.read(0, len)))
    parser.onHeaders(() => parseHeaders(context))
    parser.onRequest(() => onRequest(context))
    parser.reset(REQUEST, client)
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onDrain(() => client.resume())
    client.onClose(() => freeContext(context))
    client.onEnd(() => client.close())
    client.setNoDelay(false)
}

let server

async function boot(args) {
    buffers.out.write(r200, 0)
    server = new Socket(TCP)
    server.onConnect(onClient)
    const r = server.listen(HTTPD_LISTEN_ADDRESS || '0.0.0.0', parseInt(HTTPD_LISTEN_PORT || 3000, 10))
    if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)
}

boot(args.slice(2)).catch(err => print(err.message))
