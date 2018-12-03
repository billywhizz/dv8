const { UV_TTY_MODE_NORMAL, TTY } = module('tty', {})
const { Hmac } = module('openssl', {})
const buf2hex = (ab, len) => Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
const stdin = new TTY(0)
const buf = Buffer.alloc(64 * 1024)
const hmac = new Hmac()
hmac.setup('sha512', 'mybigsecret', buf, buf)
hmac.create()
stdin.setup(buf, UV_TTY_MODE_NORMAL)
stdin.onEnd(() => {
  const hex = buf2hex(buf.bytes, hmac.digest())
  print(hex)
  if (process.args.length > 2) {
    if (process.args[2] !== hex) {
      print('signature does not match')
      return
    }
    print('signature matches')
  }
})
stdin.onRead(len => hmac.update(len))
stdin.resume()
