const { UDP } = dv8.library('udp')

const opcode = {
  QUERY: 0,
  IQUERY: 1,
  STATUS: 2
}

const qtype = {
  A: 1,
  NS: 2,
  MD: 3,
  MF: 4,
  CNAME: 5,
  SOA: 6,
  MB: 7,
  MG: 8,
  MR: 9,
  NULL: 10,
  WKS: 11,
  PTR: 12,
  HINFO: 13,
  MINFO: 14,
  MX: 15,
  TXT: 16,
  // Additional
  AXFR: 252,
  MAILB: 253,
  MAILA: 254,
  ANY: 255
}

const qclass = {
  IN: 1,
  CS: 2,
  CH: 3,
  HS: 4,
  ANY: 255
}

const rcode = {
  NOERROR: 0,
  FORMAT: 1,
  SERVER: 2,
  NAME: 3,
  NOTIMPL: 4,
  REFUSED: 5
}

const types = { opcode, qtype, qclass, rcode }

function readName (offset, buf, view) {
  let name = []
  let qnameSize = view.getUint8(offset++)
  while (qnameSize) {
    if ((qnameSize & 192) === 192) {
      let off = (qnameSize - 192) << 8
      off += view.getUint8(offset++)
      name = name.concat(readName(off, buf, view))
      qnameSize = 0
    } else {
      name.push(buf.read(offset, qnameSize))
      offset += qnameSize
      qnameSize = view.getUint8(offset++)
    }
  }
  return name
}

const parseMessage = (buf, len) => {
  const bytes = new Uint8Array(buf.bytes)
  const view = new DataView(buf.bytes)
  const id = view.getUint16(0)
  const flags = view.getUint16(2)
  const QR = (flags >> 15) & 0b1
  const opCode = (flags >> 11) & 0b1111
  const AA = (flags >> 10) & 0b1
  const TC = (flags >> 9) & 0b1
  const RD = (flags >> 8) & 0b1
  const RA = (flags >> 7) & 0b1
  const Z = (flags >> 4) & 0b111
  const RCODE = flags & 0b1111
  const qcount = view.getUint16(4)
  const ancount = view.getUint16(6)
  const nscount = view.getUint16(8)
  const arcount = view.getUint16(10)
  const question = []
  const answer = []
  const authority = []
  const additional = []
  const start = 12
  let off = start
  let i = off
  let counter = qcount
  while (counter--) {
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
  counter = ancount
  while (counter--) {
    const next = view.getUint16(off)
    const offset = next & 0b11111111111111
    const name = readName(offset, buf, view)
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
      const cname = readName(off, buf, view)
      answer.push({ name, cname, qtype, qclass, ttl })
    } else if (qtype === 1) {
      answer.push({ name, qtype, qclass, ttl, ip: bytes.slice(off, off + rdLength) })
    }
    off += rdLength
  }
  return { bytes: bytes.slice(0, len), qcount, nscount, ancount, arcount, id, flags, QR, opCode, AA, TC, RD, RA, Z, RCODE, question, answer, authority, additional }
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

const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]

async function lookup ({ query = 'www.google.com', address = '8.8.8.8', port = 53 }) {
  return new Promise(resolve => {
    const sock = new UDP()
    sock.setup(rb, wb)
    sock.onMessage((len, address, port) => {
      sock.stop()
      const message = { length: len, address, port, message: parseMessage(rb, len) }
      resolve(message)
      sock.close()
    })
    sock.bind('0.0.0.0', 0)
    sock.start()
    const len = createMessage(query, wb, 1)
    sock.send(len, address, port)
  })
}

const qtypes = {}
Object.keys(types.qtype).forEach(k => {
  qtypes[types.qtype[k]] = k
})
const qclasses = {}
Object.keys(types.qclass).forEach(k => {
  qclasses[types.qclass[k]] = k
})
const opcodes = {}
Object.keys(types.opcode).forEach(k => {
  opcodes[types.opcode[k]] = k
})
const rcodes = {}
Object.keys(types.rcode).forEach(k => {
  rcodes[types.rcode[k]] = k
})

module.exports = { lookup, types, qtypes, qclasses, opcodes, rcodes }
