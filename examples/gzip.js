const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { ZLib, ZLIB_MODE_GZIP, Z_NO_FLUSH, Z_FINISH } = module('libz', {})

function dumpHandles () {
  print(JSON.stringify(process.activeHandles(), null, '  '))
}

process.onExit = dumpHandles

const stdin = new TTY(0)
const stdout = new TTY(1)
const BUFFER_SIZE = 32 * 1024
let remaining = BUFFER_SIZE
const raw = Buffer.alloc(BUFFER_SIZE)
const compressed = Buffer.alloc(BUFFER_SIZE)
const deflate = new ZLib(ZLIB_MODE_GZIP)

let bytesWritten = 0

deflate.setup(raw, compressed, 9)

stdin.setup(raw, UV_TTY_MODE_RAW)
stdout.setup(compressed, UV_TTY_MODE_RAW)

stdin.onEnd(() => stdin.close())

stdin.onRead(len => {
  remaining = deflate.write(len, Z_NO_FLUSH, remaining)
  print(`gzip.onRead: ${remaining}`)
  if (remaining < 0) throw new Error('Buffer Overrun')
  if (remaining < 1024) {
    const len = BUFFER_SIZE - remaining
    const r = stdout.write(len)
    remaining = BUFFER_SIZE
    if (r < 0) return stdout.close()
    bytesWritten += len
    if (r < len) stdin.pause()
  }
})

stdin.onClose(() => stdout.close())
stdout.onDrain(() => stdin.resume())

stdin.onClose(() => {
  print(`gzip.written: ${bytesWritten}`)
})

stdin.onEnd(() => {
  remaining = deflate.write(0, Z_FINISH, remaining)
  print(`gzip.onEnd: ${remaining}`)
  deflate.end()
  let len = BUFFER_SIZE - remaining
  if (len > 0) {
    const r = stdout.write(len)
    remaining = BUFFER_SIZE
    if (r < 0) return stdout.close()
    bytesWritten += len
    if (r < len) {
      stdout.onDrain(() => stdin.close())
      return stdin.pause()
    }
  }
  stdin.close()
})

stdin.resume()
