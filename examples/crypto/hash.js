const { Hash } = module('openssl', {})

function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

const [ wb, rb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
const { bytes } = rb
const hash = new Hash()
hash.setup('md5', wb, rb)

hash.create(wb.write('hello'))
print(buf2hex(bytes, hash.digest()))

hash.create()
hash.update(wb.write('hello'))
print(buf2hex(bytes, hash.digest()))

hash.create(wb.write('hello'))
print(buf2hex(bytes, hash.digest()))

hash.create()
hash.update(wb.write('h'))
hash.update(wb.write('e'))
hash.update(wb.write('l'))
hash.update(wb.write('l'))
hash.update(wb.write('o'))
print(buf2hex(bytes, hash.digest()))

hash.setup('sha256', wb, rb)
hash.create(wb.write('hello'))
print(buf2hex(bytes, hash.digest()))

hash.create(wb.write('hello'))
print(buf2hex(bytes, hash.digest()))

hash.setup('sha1', wb, rb)
hash.create(wb.write('hello'))
print(buf2hex(bytes, hash.digest()))

hash.create()
hash.update(wb.write('h'))
hash.update(wb.write('e'))
hash.update(wb.write('l'))
hash.update(wb.write('l'))
hash.update(wb.write('o'))
print(buf2hex(bytes, hash.digest()))
