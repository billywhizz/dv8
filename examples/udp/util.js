const b2hex = (buf, len) => {
  return Array.prototype.map.call((new Uint8Array(buf.bytes)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

const hex2b = (hex, buf) => {
  const bytes = new Uint8Array(buf.bytes)
  let len = 0
  hex.match(/.{1,2}/g).forEach(byte => {
    bytes[len++] = parseInt(byte, 16)
  })
  return len
}

module.exports = { b2hex, hex2b }
