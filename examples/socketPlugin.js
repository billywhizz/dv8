const { Socket, TCP } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const sock = new Socket(TCP)
const BUFFER_SIZE = 4 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE
const { HTTPD_LISTEN_ADDRESS, HTTPD_LISTEN_PORT } = env

const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
const r200len = r200.length

const contexts = []

function createContext() {
    if (contexts.length) return contexts.shift()
    const context = {
        in: createBuffer(BUFFER_SIZE),
        out: createBuffer(BUFFER_SIZE),
        work: createBuffer(BUFFER_SIZE),
        parser: new HTTPParser()
    }
    context.in.bytes = new Uint8Array(context.in.bytes)
    context.dv = new DataView(context.work.bytes)
    context.parser.setup(context.in, context.work)
    return context
}

function freeContext(context) {
    contexts.push(context)
}

function onClient(fd) {
    const client = new Socket(TCP)
    const context = createContext()
    const { parser } = context
    parser.onRequest(() => {
        const r = client.write(r200len)
        if (r === r200len) return
        if (r < r200len && client.queueSize() >= MAX_BUFFER) return client.pause()
        if (r < 0) client.close()
    })
    parser.onError((errno, message) => client.close())
    parser.reset(REQUEST, client)
    client.setup(fd, context.in, context.out)
    client.onDrain(() => client.resume())
    client.onClose(() => freeContext(context))
    //client.onError(err => print(`client.error: ${err} ${client.error(err)}`))
    client.onEnd(() => client.close())
    client.setNoDelay(false)
    context.out.write(r200, 0)
}

sock.onConnect(onClient)

const r = sock.listen(HTTPD_LISTEN_ADDRESS || '0.0.0.0', parseInt(HTTPD_LISTEN_PORT || 3000, 10))
if (r !== 0) throw new Error(`Listen Error: ${r} ${sock.error(r)}`)
