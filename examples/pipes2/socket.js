require('./base.js')
const { Socket, TCP, UNIX } = module('socket', {})

const server = new Socket(UNIX)
const BUFFER_SIZE = 4096
const MAX_BUFFER = 4 * BUFFER_SIZE

function createBuffer(size) {
    const buf = new Buffer()
    buf.bytes = buf.alloc(size) // buf.bytes is an instance of ArrayBuffer
    return buf
}

server.onConnect(fd => {
    print('server.connect')
    const client = new Socket(UNIX)
    client.in = createBuffer(BUFFER_SIZE)
    client.out = createBuffer(BUFFER_SIZE)
    client.setup(fd, client.in, client.out)
    client.onRead(len => {
        //print(`server.onRead: ${len}`)
    })
    client.onWrite((len, status) => {
        print(`server.onWrite: ${len}, ${status}`)
    })
    client.onDrain(() => {
        print(`server.onDrain`)
    })
    client.onError(err => {
        print(`server.onError: ${err}`)
    })
    client.onEnd(() => {
        print(`server.onEnd`)
    })
    client.onClose(() => {
        print(`server.onClose`)
    })
})

const client = new Socket(UNIX)
client.onConnect(fd => {
    print('client.connect')
    client.in = createBuffer(BUFFER_SIZE)
    client.out = createBuffer(BUFFER_SIZE)
    client.setup(fd, client.in, client.out)
    client.onRead(len => {
        print(`client.onRead: ${len}`)
    })
    client.onWrite((len, status) => {
        //print(`client.onWrite: ${len}, ${status}`)
    })
    client.onDrain(() => {
        //print(`client.onDrain`)
        client.timer = setTimeout(flood, 0)
    })
    client.onError(err => {
        print(`client.onError: ${err}`)
    })
    client.onEnd(() => {
        print(`client.onEnd`)
        client.close()
    })
    client.onClose(() => {
        print(`client.onClose`)
    })
    function flood() {
        let written = client.write(0, BUFFER_SIZE)
        while (written === BUFFER_SIZE && client.queueSize() < MAX_BUFFER) {
            written = client.write(0, BUFFER_SIZE)
        }
        //print(`written: ${written}, error: ${client.error(written)}`)
        if (written < 0) {
            //client.close()
        }
    }
    flood()
})

server.listen('/tmp/foo.sock')
client.connect('/tmp/foo.sock')