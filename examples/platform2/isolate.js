const { Socket, UNIX } = module('socket', {})
const { Parser } = require('./lib/ipc.js')
const peer = new Socket(UNIX)

peer.onConnect(fd => {
  const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
  const parser = new Parser(rb, wb)
  peer.setup(fd, rb, wb)
  peer.onRead(len => {
    parser.read(len)
  })
  peer.onEnd(() => peer.close())
  parser.onMessage = message => {
    console.log(`thread (${process.threadId}) recv from ${message.threadId}`)
    const o = JSON.parse(message.payload)
    console.log(JSON.stringify(o))
    if (o.shutdown) {
      setTimeout(() => peer.close(), 3000)
    }
  }
  peer.write(parser.write({}))
})

peer.connect(`./${process.PID}.sock`)
