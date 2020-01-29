const { library, args, print } = dv8

function mbedtls () {
  const { Hash, Hmac, Crypto } = library('mbedtls')
  const { MD2, MD4, MD5, SHA1, SHA256, SHA512, RIPEMD160 } = Crypto

  const digest = new Array(32)

  Hash.md5('hello', digest)
  let hash = digest.map(v => v.toString(16).padStart(2, '0')).join('')
  if (hash !== '5d41402abc4b2a76b9719d911017c592') {
    throw new Error('Does Not Match')
  }

  Hash.md5(args[2] || 'hello', digest)
  hash = digest.slice(0, 16).map(v => ('00' + v.toString(16)).slice(-2)).join('')
  print(hash)

  Hash.sha256(args[2] || 'hello', digest)
  hash = digest.slice(0, 32).map(v => ('00' + v.toString(16)).slice(-2)).join('')
  print(hash)

  Hmac.sha256('key', 'secret', digest)
  hash = digest.slice(0, 32).map(v => ('00' + v.toString(16)).slice(-2)).join('')
  print(hash)

  Crypto.hash(MD5, args[2] || 'hello', digest)
  hash = digest.slice(0, 16).map(v => ('00' + v.toString(16)).slice(-2)).join('')
  print(hash)

  Crypto.hash(SHA256, args[2] || 'hello', digest)
  hash = digest.slice(0, 32).map(v => ('00' + v.toString(16)).slice(-2)).join('')
  print(hash)

  Crypto.hmac(SHA256, 'key', 'secret', digest)
  hash = digest.slice(0, 32).map(v => ('00' + v.toString(16)).slice(-2)).join('')
  print(hash)
}

function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

function openssl () {
  const { Hmac, Hash } = library('openssl')
  const [wb, rb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
  const { bytes } = rb
  const hmac = new Hmac()
  const hash = new Hash()
  hash.setup('md5', wb, rb)
  hash.create(wb.write('hello'))
  print(buf2hex(bytes, hash.digest()))
  hash.create()
  hash.setup('sha256', wb, rb)
  hash.create(wb.write('hello'))
  print(buf2hex(bytes, hash.digest()))
  hmac.setup('sha256', 'key', wb, rb)
  hmac.create(wb.write('secret'))
  print(buf2hex(bytes, hmac.digest()))

}

mbedtls()
openssl()
