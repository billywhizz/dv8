const { OS } = module('os', {})
const { Socket, TCP } = module('socket', {})
const { SecureContext, SecureSocket } = module('openssl', {})

const IN_BUFFER_SIZE = 16 * 1024
const OUT_BUFFER_SIZE = 16 * 1024

const SIGTERM = 15
const os = new OS()

function terminateHandler(signum) {
    print('SIGTERM')
    server.close()
    return 0
}

os.onSignal(terminateHandler, SIGTERM)

const buffers = { in: createBuffer(IN_BUFFER_SIZE), out: createBuffer(OUT_BUFFER_SIZE) }

function onClient(fd) {
    const client = new Socket(TCP)
    const secureClient = new SecureSocket()
    client.setup(fd, buffers.in, buffers.out)
    client.onDrain(() => client.resume())
    client.onClose(() => secureClient.finish())
    client.onEnd(() => client.close())
    secureClient.onRead((len) => {
        print(buffers.in.read(0, len))
        buffers.in.copy(buffers.out, len)
        const r = secureClient.write(len)
        if (r < 0) return client.close()
        if (r < r200.length && client.queueSize() >= MAX_BUFFER) client.pause()
    })
    client.setNoDelay(true)
    client.setKeepAlive(true, 3000)
    secureClient.setup(context, client)
}
const server = new Socket(TCP)
server.onConnect(onClient)
const context = new SecureContext()
context.setup('./cert.pem', './key.pem') // server socket is default
const r = server.listen('0.0.0.0', 3000)
if (r !== 0) throw new Error(`Listen Error: ${r} ${server.error(r)}`)
server.onClose(() => {
    print('server.onClose')
    context.finish()
})