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
const requests = []
const buffers = { in: createBuffer(IN_BUFFER_SIZE), out: createBuffer(OUT_BUFFER_SIZE) }

function Request(address, major, minor, method, upgrade, keepalive, url, headers) {
    this.address = address
    this.major = major
    this.minor = minor
    this.method = method
    this.upgrade = upgrade
    this.keepalive = keepalive
    this.url = url
    this.headers = headers
}

Request.prototype.address = ''
Request.prototype.major = 0
Request.prototype.minor = 0
Request.prototype.method = 0
Request.prototype.upgrade = 0
Request.prototype.keepalive = 0
Request.prototype.url = ''
Request.prototype.headers = ''

function createRequest(context) {
    const { client, bytes, view, work } = context
    const { address } = client
    const urlLength = view.getUint16(5)
    const headerLength = view.getUint16(7)
    const str = work.read(STRING_START, STRING_START + urlLength + headerLength)
    if (requests.length) {
        const request = requests.shift()
        request.address = address
        request.major = bytes[0]
        request.minor = bytes[1]
        request.method = bytes[2]
        request.upgrade = bytes[3]
        request.keepalive = bytes[4]
        request.url = str.substring(0, urlLength)
        request.headers = str.substring(urlLength, urlLength + headerLength)
        return request
    }
    return new Request(address, bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], str.substring(0, urlLength), str.substring(urlLength, urlLength + headerLength))
}

function createContext() {
    if (contexts.length) return contexts.shift()
    const work = createBuffer(WORK_BUFFER_SIZE)
    const parser = new HTTPParser()
    const bytes = new Uint8Array(work.bytes)
    const view = new DataView(work.bytes)
    const context = { in: buffers.in, out: buffers.out, work, parser, bytes, view, queue: [] }
    context.parser.setup(context.in, work)
    return context
}

function onHeaders(context) {
    context.queue.push(createRequest(context))
}

function onRequest(context) {
    const { client } = context
    const request = context.queue.shift()
    //print(JSON.stringify(request, null, '  '))
    requests.push(request)
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

const timer = setInterval(() => {
    print(JSON.stringify({ requests: requests.length, contexts: contexts.length }))
}, 1000)

const server = new Socket(TCP)
buffers.out.write(r200, 0)
server.onConnect(onClient)
const r = server.listen(HTTPD_LISTEN_ADDRESS || '0.0.0.0', parseInt(HTTPD_LISTEN_PORT || 3000, 10))
if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)
