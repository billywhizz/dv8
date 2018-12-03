const { UV_TTY_MODE_NORMAL, TTY } = module('tty', {})
const { Hmac } = module('openssl', {})
const { start, stop } = require('./meter.js')
const alg = process.args[2] || 'sha256'
const algorithms = [ 'md4', 'md5', 'ripemd160', 'sha', 'sha1', 'sha224', 'sha256', 'sha384', 'sha512', 'whirlpool' ]
if (!algorithms.includes(alg)) throw new Error(`Algorithm ${alg} not supported`)
const sig = process.args[3] || 'mybigsecret'
function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}
const stdin = new TTY(0)
const BUFFER_SIZE = 64 * 1024
const buf = Buffer.alloc(BUFFER_SIZE)
const { bytes } = buf
const hmac = new Hmac()
hmac.setup(alg, sig, buf, buf)
hmac.create()
stdin.name = 'count.stdin'
stdin.setup(buf, UV_TTY_MODE_NORMAL)
stdin.onEnd(() => stdin.close())
stdin.onClose(() => {
  stop(stdin)
  print(buf2hex(bytes, hmac.digest()))
})
stdin.onRead(len => {
  hmac.update(len)
})
stdin.resume()
start(stdin)
