const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})
const { setSecure, addContext } = require('./lib/tls.js')

let currentTime = (new Date()).toUTCString()

const sleep = ms => new Promise(ok => setTimeout(ok, ms))

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

const onRequest = async context => {
    const { client, out, request } = context
    const { url, headers, major, minor, method, upgrade, keepalive } = request
    const parts = url.split('/').filter(v => v)
    let size = 1024
    if (parts.length) {
        size = size * parseInt(parts.pop(), 10)
    }
    let chunkSize = 4096
    const keepAlive = (request.keepalive === 1)
    let len = out.write(`HTTP/1.1 200 OK\r\nServer: dv8\r\nDate: ${currentTime}\r\nContent-Length: ${size}\r\n\r\n`, 0)
    const r = client.write(len)
    if (r < 0) {
        client.close()
        return
    }
    if (r < len) {
        client.pause()
        client.paused = true
    }
    let done = 0
    const bytes = new Uint8Array(out.bytes)
    bytes.fill(32)
    function next() {
        //print(done)
        if (request.timer) clearTimeout(request.timer)
        if (client.paused) {
            request.timer = setTimeout(next, 10)
            return
        }
        if (size === done) {
            if (!keepAlive) {
                //client.close()
            }
            return
        }
        if (size - done < chunkSize) chunkSize = size - done
        const r = client.write(chunkSize)
        if (r < 0) {
            print('moo')
            client.close()
            return
        }
        if (r < len) {
            print('ass')
            client.pause()
            client.paused = true
            done += r
            request.timer = setTimeout(next, 10)
            return
        }
        done += r
        nextTick(next)
    }
    nextTick(next)
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
    client.pause()
    client.address = client.remoteAddress()
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.onDrain(() => {
        client.paused = false
        client.resume()
    })
    client.setNoDelay(true)
    client.setKeepAlive(true, 3000)
    setSecure(client)
    parser.reset(REQUEST, client)
    client.resume()
}

addContext('dv8.billywhizz.io')
const timer = setInterval(() => {
    currentTime = (new Date()).toUTCString()
}, 1000)
const contexts = []
const server = new Socket(TCP)
server.onConnect(onClient)
const r = server.listen('0.0.0.0', 3000)
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
