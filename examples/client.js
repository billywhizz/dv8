require('./base.js')
const { Socket } = module('socket', {})

const context = {
    in: new Buffer(),
    out: new Buffer()
}
context.in.ab = new Uint8Array(context.in.alloc(4 * 1024))
context.out.ab = new Uint8Array(context.out.alloc(4 * 1024))

const backend = new Socket(0)
let timer
const rGET = 'GET / HTTP/1.1\r\n\r\n'
const len = rGET.length

backend.onConnect(fd => {
    backend.setup(fd, context.in, context.out)
    backend.push(fd, rGET, 0)
    backend.setNoDelay(fd, false)
    timer = setInterval(() => {
        backend.write(fd, 0, len)
    }, 1000)
    backend.resume(fd)
})

//backend.onWrite((fd, index, len) => {
//    print(`onWrite: ${index}: ${len}`)
//})

backend.onData((fd, len) => {
    print(backend.pull(fd, 0, len))
})

backend.onClose(fd => {
    print('closed')
    clearTimeout(timer)
})

const r = backend.connect('127.0.0.1', 3000)
print(`connect: ${r}`)
