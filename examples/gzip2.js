const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { ZLib, ZLIB_MODE_GZIP, ZLIB_MODE_GUNZIP, Z_NO_FLUSH, Z_FINISH } = module('libz', {})
const { buf2hex } = require('./lib/util.js')

let remaining = 0
const stdin = new TTY(0)
const BUFFER_SIZE = 4 * 1024
const raw = Buffer.alloc(BUFFER_SIZE)
const compressed = Buffer.alloc(BUFFER_SIZE)
const deflate = new ZLib(ZLIB_MODE_GZIP)

deflate.setup(raw, compressed, 9)

stdin.setup(raw, UV_TTY_MODE_RAW)

stdin.onEnd(() => stdin.close())

stdin.onRead(len => {
  remaining = deflate.write(len, Z_NO_FLUSH)
})

stdin.onClose(() => {
  remaining = deflate.write(0, Z_FINISH)
  //print(remaining)
  let size = BUFFER_SIZE - remaining
  //print(`compressed size: ${size}`)
  deflate.end()
  //print(buf2hex(compressed, size))
  const inflate = new ZLib(ZLIB_MODE_GUNZIP)
  const decompressed = Buffer.alloc(BUFFER_SIZE)
  inflate.setup(compressed, decompressed, 9)
  size = BUFFER_SIZE - inflate.write(size, Z_FINISH)
  inflate.end()
  //print(decompressed.read(0, size))
  //print(`decompressed size: ${size}`)
})

stdin.resume()
