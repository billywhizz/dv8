const { Hash } = module('openssl', {})

function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

const [ wb, rb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
const { bytes } = rb
const hash = new Hash()

function test (alg) {
  hash.setup(alg, wb, rb)
  hash.create(wb.write('hello'))
  print(buf2hex(bytes, hash.digest()))
}

/*
md4
md5
ripemd160
sha
sha1
sha224
sha256
sha384
sha512
whirlpool
*/
test(process.args[2] || 'sha256')
