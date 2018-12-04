const { UDP } = module('udp', {})

class Client {
  constructor (address, port) {
    const requests = {}
    const sock = new UDP()
    const buf = Buffer.alloc(65536)
    const view = new DataView(buf.bytes)
    const bytes = new Uint8Array(buf.bytes)
  
    this.requests = requests
    this.address = address
    this.port = port
    this.id = (Math.floor(Math.random() * 10) & 0xffff)
    this.sock = sock
    this.buf = buf
    this.view = view
    this.bytes = bytes

    const voidFn = () => {}
    sock.setup(buf, buf)
    sock.onClose(voidFn)
    sock.onSend(voidFn)

    sock.onMessage((len, address, port) => {
      const message = this.parseMessage(len)
      const resolve = requests[message.id]
      if (resolve) {
        delete requests[message.id]
        resolve({ address, port, message })
      }
    })

    sock.bind('0.0.0.0', 0)
    sock.start()
  }

  stop () {
    return this.sock.start()
  }

  start () {
    return this.sock.stop()
  }

  close () {
    return this.sock.close()
  }

  readSection (start, off, sections) {
    const { view, buf } = this
    let qnameSize = 0
    while (1) {
      qnameSize = view.getUint8(off++)
      if (qnameSize === 0) break
      sections.push(buf.read(off, qnameSize))
      off += qnameSize
    }
    return off
  }

  parseMessage (len) {
    const { bytes, view } = this
    const id = view.getUint16(0)
    const flags = view.getUint16(2)
    const question = []
    const answer = []
    const start = 12
    let off = start
    let qcount = view.getUint16(4)
    while (qcount--) {
      const sections = []
      off = this.readSection(start, off, sections)
      question.push({ qtype: view.getUint16(off), qclass: view.getUint16(off + 2), name: sections })
      off += 4
    }
    let ancount = view.getUint16(6)
    while (ancount--) {
      const name = view.getUint16(off)
      const sections = []
      // TODO: check first 2 bits - if 0b11 then this is a reference to a field elswhere in the message
      this.readSection(start, name & 0b11111111111111, sections)
      off += 2
      const qtype = view.getUint16(off)
      off += 2
      const qclass = view.getUint16(off)
      off += 2
      const ttl = view.getUint32(off)
      off += 4
      const rdLength = view.getUint16(off)
      off += 2
      if (qtype === 5) {
        let start = off
        const cnames = []
        off = this.readSection(start, off, cnames)
        answer.push({ name: sections, qtype, qclass, ttl, cnames })
      } else if (qtype === 1) {
        answer.push({ name: sections, qtype, qclass, ttl, ip: bytes.slice(off, off + rdLength) })
        off += rdLength
      }
    }
    return { id, flags, QR: (flags >> 15) & 0x1, opCode: (flags >> 11) & 0b1111, AA: (flags >> 10) & 0b1, TC: (flags >> 9) & 0b1, RD: (flags >> 8) & 0b1, RA: (flags >> 7) & 0b1, Z: (flags >> 6) & 0b111, RCODE: (flags >> 4) & 0b1111, question, answer }
  }

  createMessage (domain, qtype = 1, qclass = 1) {
    const { buf, view, bytes, id } = this
    view.setUint16(0, id)
    view.setUint16(2, 0b0000000101000000)
    view.setUint16(4, 1)
    view.setUint16(6, 0)
    view.setUint16(8, 0)
    view.setUint16(10, 0)
    let off = 12
    const parts = domain.split('.')
    for (const part of parts) {
      view.setUint8(off++, part.length)
      buf.write(part, off)
      off += part.length
    }
    bytes[off++] = 0
    view.setUint16(off, qtype)
    off += 2
    view.setUint16(off, qclass)
    off += 2
    return off
  }

  query (name) {
    const { requests, sock, address, port } = this
    const message = this.createMessage(name)
    const promise = new Promise(resolve => {
      requests[this.id++] = resolve
      sock.send(message, address, port)
    })
    return promise
  }
}

async function run () {
  const dns = new Client('8.8.8.8', 53)
  const host = process.args[2] || 'www.example.com'
  const result = await dns.query(host)
  console.log(JSON.stringify(result, null, '  '))
  dns.stop()
  dns.close()
}

run().catch(err => console.log(err.toString()))
