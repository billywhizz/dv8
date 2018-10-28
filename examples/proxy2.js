const { Socket, TCP } = module('socket', {})
const BUFFER_SIZE = 64 * 1024

const { PROXY_LOCAL_ADDRESS, PROXY_LOCAL_PORT, PROXY_REMOTE_ADDRESS, PROXY_REMOTE_PORT } = env

const contexts = []

function createContext() {
    if (!contexts.length) {
        return { in: createBuffer(BUFFER_SIZE), out: createBuffer(BUFFER_SIZE) }
    }
    return contexts.shift()
}

function freeContext(context) {
    contexts.push(context)
}

function createBuffer(size) {
    const buf = new Buffer()
    buf.bytes = buf.alloc(size)
    return buf
}

const onRead = (src, dst) => len => {
    const r = dst.write(len)
    if (r < 0) return dst.close()
    if (r === 0) return src.pause()
    if (r < len && src.queueSize() >= BUFFER_SIZE * 4) src.pause()
}

function pipe(src, dst) {
    src.onRead(onRead(src, dst))
    src.onEnd(() => src.close())
    src.onError((err, message) => print(`error: ${err}\n${message}`))
    src.onDrain(() => dst.resume())
}

function onConnect(fd) {
    const remote = new Socket(TCP)
    const local = new Socket(TCP)
    const context = createContext()
    local.setup(fd, context.in, context.out)
    local.pause()
    local.onClose(() => {
        local.closed = true
        if (remote.closed) {
            freeContext(context)
            return
        }
        remote.close()
    })
    remote.onClose(() => {
        remote.closed = true
        if (local.closed) {
            freeContext(context)
            return
        }
        local.close()
    })
    remote.onError((err, message) => print(`error: ${err}\n${message}`))
    const r = remote.connect(PROXY_REMOTE_ADDRESS || '127.0.0.1', PROXY_REMOTE_PORT || 3000)
    if (r !== 0) return remote.close()
    remote.onConnect(fd => {
        remote.setup(fd, context.out, context.in)
        pipe(local, remote)
        pipe(remote, local)
        local.resume()
    })
}

const server = new Socket(TCP)
server.onConnect(onConnect)
const r = server.listen(PROXY_LOCAL_ADDRESS || '0.0.0.0', PROXY_LOCAL_PORT || 3001)
if(r !== 0) print(`listen: ${r}, ${server.error(r)}`)
