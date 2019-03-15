const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const { ZLib, ZLIB_MODE_GUNZIP, Z_NO_FLUSH, Z_FINISH } = module('libz', {})

function dumpHandles () {
  print(JSON.stringify(process.activeHandles(), null, '  '))
}

process.onExit = dumpHandles

const stdin = new TTY(0)
const stdout = new TTY(1)
const BUFFER_SIZE = 64 * 1024
let remaining = BUFFER_SIZE
const compressed = Buffer.alloc(BUFFER_SIZE)
const raw = Buffer.alloc(BUFFER_SIZE)
const inflate = new ZLib(ZLIB_MODE_GUNZIP)

let bytesWritten = 0

inflate.setup(compressed, raw, 9)

stdin.setup(compressed, UV_TTY_MODE_RAW)
stdout.setup(raw, UV_TTY_MODE_RAW)

stdin.onEnd(() => stdin.close())

stdin.onRead(len => {
  remaining = inflate.write(len, Z_NO_FLUSH, remaining)
  print(`gunzip.onRead: ${remaining}`)
  if (remaining < 0) throw new Error(`Buffer Overrun: ${remaining}`)
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
  print(`gunzip.written: ${bytesWritten}`)
})

stdin.onEnd(() => {
  remaining = inflate.write(0, Z_FINISH, remaining)
  print(`gunzip.onEnd: ${remaining}`)
  inflate.end()
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
