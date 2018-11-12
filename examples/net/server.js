const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const createContext = () => {
    if (contexts.length) return contexts.shift()
    const work = createBuffer(4096)
    const parser = new HTTPParser()
    parser.setup(buffers.in, work)
    return { in: buffers.in, out: buffers.out, work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), request: { major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', body: [] }, client: null }
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
    const { client } = context
    const r = client.write(r200len)
    if (r === r200len) return
    if (r < r200len) return client.pause()
    if (r < 0) client.close()
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
    parser.reset(REQUEST, client)
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onDrain(() => client.resume())
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.setNoDelay(false)
}

const contexts = []
const buffers = { in: createBuffer(4096), out: createBuffer(4096) }
const server = new Socket(TCP)
const r200len = buffers.out.write('HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n', 0)
server.onConnect(onClient)
const r = server.listen('0.0.0.0', 3000)
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
