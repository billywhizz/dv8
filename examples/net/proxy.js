const { Socket, TCP } = module('socket', {})

const pipe = (src, dst) => {
    src.onRead(len => {
        const r = dst.write(len)
        if (r === len) return
        if (r < 0) return dst.close()
        if (r === 0 || r < len) src.pause()
    })
    src.onClose(() => dst.close())
    src.onEnd(() => src.close())
    src.onError((err, message) => print(`error: ${err}\n${message}`))
    src.onDrain(() => dst.resume())
}

const onConnect = fd => {
    const dst = new Socket(TCP)
    const src = new Socket(TCP)
    const [ rb, wb ] = [ createBuffer(16384), createBuffer(16384) ]
    src.setup(fd, rb, wb)
    src.pause()
    pipe(src, dst)
    pipe(dst, src)
    dst.onConnect(fd => {
        dst.setup(fd, wb, rb)
        src.resume()
    })
    if (dst.connect('127.0.0.1', 3000) !== 0) dst.close()
}

const server = new Socket(TCP)
server.onConnect(onConnect)
server.listen('0.0.0.0', 3001)
