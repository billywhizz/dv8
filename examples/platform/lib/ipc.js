const { Socket, UNIX } = module('socket', {})
const { Thread } = module('thread', {})

const THREAD_BUFFER_SIZE = process.env.THREAD_BUFFER_SIZE || (1 * 1024)

let next = 1

class Parser {
  constructor (rb, wb) {
    this.position = 0
    this.message = {
      opCode: 0,
      threadId: 0,
      payload: ''
    }
    this.rb = rb
    this.wb = wb
    this.view = {
      recv: new DataView(rb.bytes),
      send: new DataView(wb.bytes)
    }
    this.onMessage = () => { }
  }

  read (len, off = 0) {
    const { rb, view, message } = this
    let { position } = this
    const { recv } = view
    while (off < len) {
      if (position === 0) {
        message.threadId = recv.getUint8(off++)
        position++
      } else if (position === 1) {
        message.opCode = recv.getUint8(off++)
        position++
      } else if (position === 2) {
        message.length = recv.getUint8(off++) << 8
        position++
      } else if (position === 3) {
        message.length += recv.getUint8(off++)
        position++
        message.payload = ''
      } else {
        let toread = message.length - (position - 4)
        if (toread + off > len) {
          toread = len - off
          message.payload += rb.read(off, toread)
          position += toread
          off = len
        } else {
          message.payload += rb.read(off, toread)
          off += toread
          this.onMessage(Object.assign({}, message))
          position = 0
        }
      }
    }
    this.position = position
  }

  write (o, opCode = 1, off = 0) {
    const { wb, view } = this
    const { send } = view
    if (opCode === 1) { // JSON
      const message = JSON.stringify(o)
      const len = message.length
      send.setUint8(0, process.TID || process.PID)
      send.setUint8(1, opCode)
      send.setUint16(2, len)
      wb.write(message, 4)
      return len + 4
    } else if (opCode === 2) { // String
      const len = o.length
      send.setUint8(0, process.TID || process.PID)
      send.setUint8(1, opCode)
      send.setUint16(2, len)
      wb.write(o, 4)
      return len + 4
    }
    return 0
  }
}

if (process.TID) {
  // we are in a thread context
  const peer = new Socket(UNIX)
  peer.onConnect(fd => {
    const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
    const parser = new Parser(rb, wb)
    peer.setup(fd, rb, wb)
    peer.onRead(len => {
      parser.read(len)
    })
    peer.onEnd(() => peer.close())
    parser.onMessage = message => peer.onMessage(JSON.parse(message.payload))
    process.send = o => peer.write(parser.write(o))
  })
  peer.connect(`./${process.PID}.sock`)
  process.onMessage = fn => {
    peer.onMessage = fn
  }
  peer.unref()
} else {
  // we are in the main process context
  const listener = new Socket(UNIX)
  listener.onConnect(fd => {
    const peer = new Socket(UNIX)
    const [rb, wb] = [Buffer.alloc(16384), Buffer.alloc(16384)]
    const parser = new Parser(rb, wb)
    peer.setup(fd, rb, wb)
    peer.onRead(len => {
      parser.read(len)
    })
    peer.onEnd(() => peer.close())
    parser.onMessage = message => {
      const o = JSON.parse(message.payload)
      const thread = process.threads[message.threadId]
      thread.send = o => peer.write(parser.write(o))
      thread._onMessage(o)
    }
    peer.unref()
  })
  process.spawn = (fun, onComplete) => {
    const start = process.hrtime()
    const thread = new Thread()
    thread.buffer = Buffer.alloc(THREAD_BUFFER_SIZE)
    const view = new DataView(thread.buffer.bytes)
    thread.view = view
    thread.id = next++
    view.setUint8(0, thread.id)
    const envJSON = JSON.stringify(process.env)
    const argsJSON = JSON.stringify(process.args)
    view.setUint32(1, envJSON.length)
    thread.buffer.write(envJSON, 5)
    view.setUint32(envJSON.length + 5, argsJSON.length)
    thread.buffer.write(argsJSON, envJSON.length + 9)
    process.threads[thread.id] = thread
    thread.send = () => {}
    thread.onMessage = fn => {
      thread._onMessage = fn
    }
    thread._onMessage = message => {
      console.log(`thread (${process.TID}) recv from ${message.threadId}`)
      const o = JSON.parse(message.payload)
      console.log(JSON.stringify(o))
    }
    thread.start(fun, (err, status) => {
      const finish = process.hrtime()
      const ready = view.getBigUint64(0)
      thread.time = (finish - start) / 1000n
      thread.boot = (ready - start) / 1000n
      onComplete({ err, thread, status })
    }, thread.buffer)
    return thread
  }
  listener.listen(`./${process.PID}.sock`)
  listener.unref()
}
module.exports = {}
