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
    const parser = new HTTPParser()
    const context = { in: buffers.in, out: buffers.out, work, parser }
    context.parser.setup(context.in, work)
    return context
}

function onClient(fd) {
    const client = new Socket(TCP)
    const context = createContext()
    const { work, parser } = context
    const body = []
    context.client = client
    parser.onBody(len => body.push(work.read(0, len)))
    parser.onRequest(() => {
        const r = client.write(r200len)
        if (r === r200len) return
        if (r < r200len) return client.pause()
        if (r < 0) client.close()
    })
    parser.reset(REQUEST, client)
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onDrain(() => client.resume())
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.setNoDelay(false)
}

const timer = setInterval(() => {
    print(JSON.stringify(cpuUsage()))
}, 1000)
const server = new Socket(TCP)
buffers.out.write(r200, 0)
server.onConnect(onClient)
const r = server.listen(HTTPD_LISTEN_ADDRESS || '0.0.0.0', parseInt(HTTPD_LISTEN_PORT || 3000, 10))
if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)
