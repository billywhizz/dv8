const { Socket, UNIX } = module('socket', {})

const IPC = {
  onMessage: fn
}
class Server {
  constructor (opts) {
    this.opts = opts
  }

  start () {

  }

  stop () {

  }

}

class Client {

}

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
      send.setUint8(0, process.threadId)
      send.setUint8(1, opCode)
      send.setUint16(2, len)
      wb.write(message, 4)
      return len + 4
    } else if (opCode === 2) { // String
      const len = o.length
      send.setUint8(0, process.threadId)
      send.setUint8(1, opCode)
      send.setUint16(2, len)
      wb.write(o, 4)
      return len + 4
    }
    return 0
  }
}

module.exports = { Parser, Client, Server }
