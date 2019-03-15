const { Hmac, Hash } = module('openssl', {})

function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

const b = Buffer.alloc(4096)
const { bytes } = b
const hmac = new Hmac()
hmac.setup('sha256', 'abcdefgh12345678', b)

hmac.create(b.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create()
hmac.update(b.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create(b.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create()
hmac.update(b.write('h'))
hmac.update(b.write('e'))
hmac.update(b.write('l'))
hmac.update(b.write('l'))
hmac.update(b.write('o'))
print(buf2hex(bytes, hmac.digest()))

hmac.setup('sha256', '12345678abcdefgh', b)
hmac.create(b.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create(b.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.setup('sha256', 'abcdefgh12345678', b)
hmac.create(b.write('hello'))
print(buf2hex(bytes, hmac.digest()))

hmac.create()
hmac.update(b.write('h'))
hmac.update(b.write('e'))
hmac.update(b.write('l'))
hmac.update(b.write('l'))
hmac.update(b.write('o'))
print(buf2hex(bytes, hmac.digest()))
