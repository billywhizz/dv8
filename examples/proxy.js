const { Socket, UNIX } = module('socket', {})
const { start, stop } = require('./lib/meter.js')

const server = new Socket(UNIX)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE

server.onClose(() => print('server.close'))
server.onError(err => print(`server.error: ${err}`))
server.onConnect(fd => {
    const client = new Socket(UNIX)
    client.fd = fd
    client.name = 'proxy.client'
    client.in = createBuffer(BUFFER_SIZE)
    client.out = createBuffer(BUFFER_SIZE)
    client.setup(client.fd, client.in, client.out)
    client.pause()
    start(client)
    const backend = new Socket(UNIX)
    backend.onClose(() => print('backend.close'))
    backend.onError(err => print(`backend.error: ${err}`))
    const r = backend.connect(args[3] || '/tmp/pipe.sock')
    if (r !== 0) {
        print(`backend.connect: ${r}, ${backend.error(r)}`)
        backend.close()
        client.close()
        return
    }
    backend.onConnect(fd => {
        backend.fd = fd
        backend.name = 'proxy.backend'
        backend.in = client.out
        backend.out = client.in
        backend.setup(backend.fd, backend.in, backend.out)
        start(backend)
        backend.onRead(len => {
            const r = client.write(len)
            if (r < 0) return client.close()
            if (r === 0) return backend.pause()
            if (r < len && backend.queueSize() >= MAX_BUFFER) backend.pause()
        })
        backend.onEnd(() => backend.close())
        backend.onClose(() => {
            stop(backend)
            client.close()
        })
        backend.onError((err, message) => print(`backend.error: ${err}\n${message}`))
        backend.onDrain(() => client.resume())
        client.onRead(len => {
            const r = backend.write(len)
            if (r < 0) return backend.close()
            if (r === 0) return client.pause()
            if (r < len && backend.queueSize() >= MAX_BUFFER) client.pause()
        })
        client.onEnd(() => client.close())
        client.onClose(() => {
            stop(client)
            backend.close()
        })
        client.onError(err => print(`client.error: ${err}`))
        client.onDrain(() => backend.resume())
        client.resume()
    })
})
const r = server.listen(args[2] || '/tmp/proxy.sock')
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
