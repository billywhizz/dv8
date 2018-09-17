const { Socket } = module('socket', {})

const sock = new Socket(0)
const contexts = {}
const r200 = 'HTTP/1.1 200 OK\r\nServer: foo\r\nContent-Length: 0\r\n\r\n'
sock.onConnect(fd => {
    print(`connect: ${fd}`)
    const context = contexts[fd] = {
        in: new Buffer(),
        out: new Buffer()
    }
    context.in.alloc(4096)
    context.out.alloc(4096)
    sock.setup(fd, context.in, context.out)
    sock.push(fd, r200, 0)
})

sock.onData((fd, len) => {
    print(`onData: ${len}`)
    const context = contexts[fd]
    print(context.in.pull(0, len))
    sock.write(fd, 0, r200.length)
})

print(`listen: ${sock.listen('0.0.0.0', 3000)}`)