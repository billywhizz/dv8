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

const [rb, wb] = [Buffer.alloc(16384), Buffer.alloc(16384)]
if (process.TID) {
  // we are in a thread context
  const sock = new Socket(UNIX)
  const parser = new Parser(wb, rb)
  sock.onConnect(fd => sock.setup(fd, wb, rb))
  parser.onMessage = message => sock.onMessage(message)
  process.send = o => sock.write(parser.write(o))
  sock.onRead(len => parser.read(len))
  sock.onEnd(() => sock.close())
  process.onMessage = fn => {
    sock.onMessage = fn
  }
  sock.open(process.fd)
  process.sock = sock
} else {
  // we are in the main process context
  process.spawn = (fun, onComplete) => {
    const thread = new Thread()
    thread.buffer = Buffer.alloc(THREAD_BUFFER_SIZE)
    const view = new DataView(thread.buffer.bytes)
    thread.view = view
    thread.id = next++
    view.setUint8(0, thread.id)
    const envJSON = JSON.stringify(process.env)
    const argsJSON = JSON.stringify(process.args)
    view.setUint32(5, envJSON.length)
    thread.buffer.write(envJSON, 9)
    view.setUint32(envJSON.length + 9, argsJSON.length)
    thread.buffer.write(argsJSON, envJSON.length + 13)
    process.threads[thread.id] = thread
    thread.onMessage = fn => {
      thread._onMessage = fn
    }
    thread._onMessage = message => {}
    const sock = new Socket(UNIX)
    const parser = new Parser(rb, wb)
    sock.onConnect(fd => sock.setup(fd, rb, wb))
    sock.onEnd(() => sock.close())
    sock.onRead(len => parser.read(len))
    parser.onMessage = message => thread._onMessage(message)
    thread.send = o => sock.write(parser.write(o))
    const fd = sock.open()
    view.setUint32(1, fd)
    thread.start(fun, (err, status) => onComplete({ err, thread, status }), thread.buffer)
    thread.sock = sock
    return thread
  }
}
module.exports = {}
