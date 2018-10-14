// server
const { Socket, Server, Client } = module('socket', {})

//const { SecureSocket } = module('tls', {})
//const { Request, Response } = module('http', {})

const server = new Server(0)
const clients = []

server.onConnect = client => {
    // create read and write buffers for the new client socket
    const rb = new Buffer()
    const wb = new Buffer()
    client.setup(rb, wb)

    // fired when bytes are read from the socket
    client.onRead = len => {
        const r = client.write(len)
    }

    // fired when a write request completes on the socket (buffers are flushed to kernel)
    client.onWrite = len => {
        print(`write: ${len}, queue: ${client.queueSize()}`)
    }

    client.onError = err => {

    }

    client.onClose = () => {

    }

    client.setKeepAlive(1, 1000)
    client.setNoDelay(true)

    clients.push(client)
    client.pause()
    client.resume()
}



// client
