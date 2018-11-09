const { Socket, TCP } = module('socket', {})
const { SecureContext, SecureSocket } = module('openssl', {})
const { hexy } = require('./hexy.js')

const IN_BUFFER_SIZE = 16 * 1024
const OUT_BUFFER_SIZE = 16 * 1024

const buffers = { in: createBuffer(IN_BUFFER_SIZE), out: createBuffer(OUT_BUFFER_SIZE) }

function onClient(fd) {
    const client = new Socket(TCP)
    const bytes = new Uint8Array(buffers.in.bytes)
    client.setup(fd, buffers.in, buffers.out)
    client.onDrain(() => client.resume())
    client.onClose()
    client.onEnd(() => client.close())
    client.onRead(len => print(hexy(bytes.slice(0, len))))
/*
    client.onRead(len => {
        const r = client.write(len)
        if (r < 0) return client.close()
        if (r < r200.length && client.queueSize() >= MAX_BUFFER) client.pause()
    })
*/
    client.setNoDelay(true)
    const secureClient = new SecureSocket()
    secureClient.setup(context, client)
}
const server = new Socket(TCP)
server.onConnect(onClient)
const context = new SecureContext()
context.setup('./cert.pem', './key.pem')

const r = server.listen('0.0.0.0', 3000)
if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)