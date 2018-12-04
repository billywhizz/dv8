const { UDP } = module('udp', {})

function readSection (start, off, sections, bytes, view, buf) {
  let qnameSize = 0
  while (1) {
    qnameSize = view.getUint8(off++)
    if (qnameSize === 0) break
    sections.push(buf.read(off, qnameSize))
    off += qnameSize
  }
  return off
}

const parseMessage = (buf, len) => {
  const bytes = new Uint8Array(buf.bytes)
  const view = new DataView(buf.bytes)
  const id = view.getUint16(0)
  const flags = view.getUint16(2)
  const question = []
  const answer = []
  const start = 12
  let off = start
  let qcount = view.getUint16(4)
  while (qcount--) {
    const sections = []
    off = readSection(start, off, sections, bytes, view, buf)
    question.push({ qtype: view.getUint16(off), qclass: view.getUint16(off + 2), name: sections })
    off += 4
  }
  let ancount = view.getUint16(6)
  while (ancount--) {
    const name = view.getUint16(off)
    const sections = []
    readSection(start, name & 0b11111111111111, sections, bytes, view, buf)
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
      const sections = []
      off = readSection(start, off, sections, bytes, view, buf)
      answer.push({ name: sections, qtype, qclass, ttl, sections })
    } else if (qtype === 1) {
      answer.push({ name: sections, qtype, qclass, ttl, ip: bytes.slice(off, off + rdLength) })
      off += rdLength
    }
  }
  return { id, flags, QR: (flags >> 15) & 0x1, opCode: (flags >> 11) & 0b1111, AA: (flags >> 10) & 0b1, TC: (flags >> 9) & 0b1, RD: (flags >> 8) & 0b1, RA: (flags >> 7) & 0b1, Z: (flags >> 6) & 0b111, RCODE: (flags >> 4) & 0b1111, question, answer }
}

const createMessage = (domain, buf, id, qtype = 1, qclass = 1) => {
  const view = new DataView(buf.bytes)
  const bytes = new Uint8Array(buf.bytes)
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

let gid = 0
const sock = new UDP()
const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
sock.setup(rb, wb)
sock.onClose(() => {})
sock.onSend(() => {})
sock.onMessage((len, address, port) => {
  const message = { address, port, message: parseMessage(rb, len) }
  console.log(JSON.stringify(message, null, '  '))
  sock.stop()
})
sock.bind('0.0.0.0', 0)
sock.start()
const len = createMessage(process.args[2] || 'example.com', wb)
sock.send(len, '127.0.0.1', 53)
/*
class Client {

  constuctor (ip, port) {
    this.requests = {}
    const client = this
    this.ip = ip
    this.port = port
    const sock = new UDP()
    this.sock = sock
    const buf = Buffer.alloc(65536)
    this.buf = buf
    sock.setup(buf, buf)
    sock.onClose(() => {})
    sock.onSend(() => {})
    sock.onMessage((len, address, port) => {
      if (!client.onMessage) return
      client.onMessage({ address, port, message: parseMessage(rb, len) })
    })
    sock.bind('127.0.0.1', 0)
  }

  query (name) {
    const promise = new Promise
    const message = createMessage(name, this.buf, gid)
    this.sock.send(message, '127.0.0.1', 53)
    requests[gid] = 
  }
}

async function run() {
  const dns = new Client('127.0.0.1', 53)
  const result = await dns.query('www.example.com')
  console.log(JSON.stringify(result, null, '  '))
}

run().catch(err => console.log(err.toString()))
*/
