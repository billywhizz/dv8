const { Socket, UNIX } = module('socket', {})
const { Parser } = require('./lib/ipc.js')
const { getMetrics } = require('./lib/metrics.js')
const peer = new Socket(UNIX)

peer.onConnect(fd => {
    const [ rb, wb ] = [ createBuffer(4096), createBuffer(4096) ]
    const parser = new Parser(rb, wb)
    peer.setup(fd, rb, wb)
    peer.onRead(len => {
        parser.read(len)
    })
    peer.onEnd(() => peer.close())
    parser.onMessage = message => {
        print(`thread (${threadId}) recv from ${message.threadId}`)
        const o = JSON.parse(message.payload)
        print(JSON.stringify(o))
        if (o.shutdown) {
            setTimeout(() => peer.close(), 3000)
        }
    }
    peer.write(parser.write(getMetrics()))
})

peer.connect(`./${PID}.sock`)
