const { UV_TTY_MODE_RAW, UV_TTY_MODE_NORMAL, UV_TTY_MODE_IO,  TTY } = module('tty', {})
const { Socket, TCP, UNIX } = module('socket', {})

function pipeForWriting(sockname, buf) {
    const sock = new Socket(UNIX)
    const buf = new Buffer()
    buf.alloc(bufSize)
    sock.onConnect(fd => {
    
    })
    sock.onClose(fd => {
    
    })
    sock.onError(e => {
    
    })
    sock.onDrain(fd => {
    
    })
    sock.onWrite((fd, len) => {
    
    })
    sock.onData((fd, len) => {
    
    })
    sock.onEnd(fd => {
    
    })
    sock.connect('/tmp/foo.sock')    
}

function pipeForReading(sockname, buf) {
    const sock = new Socket(UNIX)
    sock.onConnect(fd => {
    
    })
    sock.onClose(fd => {
    
    })
    sock.onError(e => {
    
    })
    sock.onDrain(fd => {
    
    })
    sock.onWrite((fd, len) => {
    
    })
    sock.onData((fd, len) => {
    
    })
    sock.onError((fd, e) => {
    
    })
    sock.onEnd(fd => {
    
    })
    sock.listen('/tmp/foo.sock')    
}

function ttyReader() {
    const stdin = new TTY(0)

    stdin.onData(len => {
    
    })
    
    stdin.onEnd(() => {
    
    })
    
    stdin.onClose(() => {
    
    })
    
    stdin.setup(b, UV_TTY_MODE_RAW)
    
    stdin.resume()
}
