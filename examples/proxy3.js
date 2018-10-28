const { Socket, TCP } = module('socket', {})

const server = new Socket(TCP)
const BUFFER_SIZE = 64 * 1024
const MAX_BUFFER = 4 * BUFFER_SIZE
const stats = new BigUint64Array(20)

function printStats(pipe) {
    pipe.stats(stats)
    const [ close, error, read, pause, data, resume, end, , , , written, incomplete, full, drain, maxQueue, alloc, free, eagain ] = stats.map(e => e.toString())
    print(JSON.stringify({
        fd: pipe.fd,
        close, error, read, pause, data, resume, end,
        written, incomplete, full, drain, maxQueue, alloc, free, eagain
    }, null, '  '))
}

server.onClose(() => print('server.close'))
server.onError(err => print(`server.error: ${err}`))
server.onConnect(fd => {
    const client = new Socket(TCP)
    client.fd = fd
    client.name = 'proxy.client'
    client.in = createBuffer(BUFFER_SIZE)
    client.out = createBuffer(BUFFER_SIZE)
    client.setup(client.fd, client.in, client.out)
    client.pause()
    const backend = new Socket(TCP)
    backend.onClose(() => print('backend.close'))
    backend.onError(err => print(`backend.error: ${err}`))
    const r = backend.connect('127.0.0.1', 3001)
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
        backend.onRead(len => {
            const r = client.write(len)
            if (r < 0) return client.close()
            if (r === 0) return backend.pause()
            if (r < len && backend.queueSize() >= MAX_BUFFER) backend.pause()
        })
        backend.onEnd(() => backend.close())
        backend.onClose(() => {
            print('backend.close')
            printStats(backend)
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
            print('client.close')
            printStats(client)
            backend.close()
        })
        client.onError(err => print(`client.error: ${err}`))
        client.onDrain(() => backend.resume())
        client.resume()
    })
})
const r = server.listen('0.0.0.0', 3000)
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
