const { UDP } = module('udp', {})
const sock = new UDP()
const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]

const b2hex = (buf, len) => {
  return Array.prototype.map.call((new Uint8Array(buf.bytes)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

const hex2b = (hex, buf) => {
  const bytes = new Uint8Array(buf.bytes)
  let len = 0
  hex.match(/.{1,2}/g).forEach(byte => (bytes[len++] = byte))
  return len
}

// https://github.com/mdns-js/node-dns-js/blob/master/lib/dnsrecord.js
let r = sock.setup(rb, wb)
print(`setup: ${r}`)

sock.onClose(() => {
  print('close')
})

sock.onSend(() => {
  print('onSend')
})

sock.onMessage((len, address, port) => {
  print(`message from ${address}:${port}, ${len}`)
  print(b2hex(rb, len))
  sock.stop()
})

sock.bind('0.0.0.0', 0)
print(`bind: ${r}`)
r = sock.start()
print(`start: ${r}`)
r = sock.send(hex2b('AAAA01000001000000000000076578616d706c6503636f6d0000010001', wb), '127.0.0.1', 53)
print(`send: ${r}`)
