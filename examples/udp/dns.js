const { UDP } = module('udp', {})

const parseMessage = (buf, len) => {
  const bytes = new Uint8Array(buf.bytes)
  const view = new DataView(buf.bytes)
  const id = view.getUint16(0)
  const flags = view.getUint16(2)
  const QR = (flags >> 15) & 0x1
  const opCode = (flags >> 11) & 0b1111
  const AA = (flags >> 10) & 0b1
  const TC = (flags >> 9) & 0b1
  const RD = (flags >> 8) & 0b1
  const RA = (flags >> 7) & 0b1
  const Z = (flags >> 6) & 0b111
  const RCODE = (flags >> 4) & 0b1111
  let qcount = view.getUint16(4)
  let ancount = view.getUint16(6)
  print(ancount)
  let nscount = view.getUint16(8)
  let arcount = view.getUint16(10)
  const question = []
  const answer = []
  const authority = []
  const additional = []
  const start = 12
  let off = start
  let i = off
  while (qcount--) {
    let size = 0
    const sections = []
    while (bytes[i++]) size++
    if (size > 0) {
      while (off - start < size) {
        const qnameSize = view.getUint8(off++)
        sections.push(buf.read(off, qnameSize))
        off += qnameSize
      }
    }
    off++
    const qtype = view.getUint16(off)
    off += 2
    const qclass = view.getUint16(off)
    off += 2
    question.push({ qtype, qclass, name: sections })
  }
  while (ancount--) {
    const name = view.getUint16(off)
    let offset = name & 0b11111111111111
    let size = 0
    let i = offset
    const sections = []
    while (bytes[i++]) size++
    if (size > 0) {
      while (offset - start < size) {
        const qnameSize = view.getUint8(offset++)
        sections.push(buf.read(offset, qnameSize))
        offset += qnameSize
      }
    }
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
      let i = off
      let start = off
      let size = 0
      const sections = []
      while (bytes[i++]) size++
      if (size > 0) {
        while (off - start < size) {
          const qnameSize = view.getUint8(off++)
          sections.push(buf.read(off, qnameSize))
          off += qnameSize
        }
      }
      off++
      answer.push({ name: sections, qtype, qclass, ttl, sections })
    } else if (qtype === 1) {
      answer.push({ name: sections, qtype, qclass, ttl, ip: bytes.slice(off, off + rdLength) })
    }
  }
  while (nscount--) {}
  while (arcount--) {}
  return { id, flags, QR, opCode, AA, TC, RD, RA, Z, RCODE, question, answer, authority, additional }
}

let gid = 0

const createMessage = (domain, buf, qtype = 1, qclass = 1) => {
  const view = new DataView(buf.bytes)
  const bytes = new Uint8Array(buf.bytes)
  const id = gid++
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

const sock = new UDP()
const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
sock.setup(rb, wb)
sock.onClose(() => print('close'))
sock.onSend(() => print('onSend'))
sock.onMessage((len, address, port) => {
  print(`message from ${address}:${port}, ${len}`)
  const message = { address, port, message: parseMessage(rb, len) }
  print(JSON.stringify(message, null, '  '))
  sock.stop()
})
sock.bind('0.0.0.0', 0)
sock.start()
const len = createMessage(process.args[2] || 'example.com', wb)
const message = parseMessage(wb, len)
print(JSON.stringify(message, null, '  '))
sock.send(len, '127.0.0.1', 53)
