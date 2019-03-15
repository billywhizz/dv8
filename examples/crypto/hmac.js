const { Hmac } = module('openssl', {})

function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

const [ wb, rb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
const { bytes } = rb
const hmac = new Hmac()
hmac.setup('sha256', 'abcdefgh12345678', wb, rb)

hmac.create(wb.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create()
hmac.update(wb.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create(wb.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create()
hmac.update(wb.write('h'))
hmac.update(wb.write('e'))
hmac.update(wb.write('l'))
hmac.update(wb.write('l'))
hmac.update(wb.write('o'))
print(buf2hex(bytes, hmac.digest()))

hmac.setup('sha256', '12345678abcdefgh', wb, rb)
hmac.create(wb.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create(wb.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.setup('sha256', 'abcdefgh12345678', wb, rb)
hmac.create(wb.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create()
hmac.update(wb.write('h'))
hmac.update(wb.write('e'))
hmac.update(wb.write('l'))
hmac.update(wb.write('l'))
hmac.update(wb.write('o'))
print(buf2hex(bytes, hmac.digest()))
