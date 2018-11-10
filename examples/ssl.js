const { Socket, TCP } = module('socket', {})
const { SecureContext, SecureSocket } = module('openssl', {})
const { hexy } = require('./hexy.js')

const IN_BUFFER_SIZE = 16 * 1024
const OUT_BUFFER_SIZE = 16 * 1024

const buffers = { in: createBuffer(IN_BUFFER_SIZE), out: createBuffer(OUT_BUFFER_SIZE) }

function onClient(fd) {
    const client = new Socket(TCP)
    const secureClient = new SecureSocket()
    const bytes = new Uint8Array(buffers.in.bytes)
    client.setup(fd, buffers.in, buffers.out)
    client.onDrain(() => client.resume())
    client.onClose(() => secureClient.finish())
    client.onEnd(() => client.close())
    client.onRead(len => {
        // these are the encryped (raw) bytes coming from client
        print(`client.onRead: ${len}`)
        print(hexy(bytes.slice(0, len)))        
    })
    secureClient.onRead((len) => {
        // these are the decrypted bytes from openssl
        print(`secureClient.onRead: ${len}`)
        print(buffers.in.read(0, len))
        // copy the bytes from in to out buffer
        buffers.in.copy(buffers.out, len)
        // write plaintext to the secureClient
        const r = secureClient.write(len)
        print(`secureClient.write: ${r}`)
        if (r < 0) return client.close()
        if (r < r200.length && client.queueSize() >= MAX_BUFFER) client.pause()
    })
    client.setNoDelay(true)
    secureClient.setup(context, client)
}
const server = new Socket(TCP)
server.onConnect(onClient)
const context = new SecureContext()
context.setup('./cert.pem', './key.pem') // server socket is default

const r = server.listen('0.0.0.0', 3000)
if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)