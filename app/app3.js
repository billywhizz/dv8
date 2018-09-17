const { Socket } = module('socket', {})

const sock = new Socket(0)
const contexts = {}

sock.onConnect(fd => {
    print(`connect: ${fd}`)
    const context = contexts[fd] = {
        in: new Buffer(),
        out: new Buffer()
    }
    context.in.alloc(4096)
    context.out.alloc(4096)
    sock.setup(fd, context.in, context.out)
})

sock.onData((fd, len) => {
    const context = contexts[fd]
    print(context.in.pull(0, len))
})

print(`listen: ${sock.listen('0.0.0.0', 3000)}`)