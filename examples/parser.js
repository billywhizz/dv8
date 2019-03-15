const { UV_TTY_MODE_NORMAL, TTY } = module('tty', {})

const stdin = new TTY(0)
const BUFFER_SIZE = 64 * 1024

const buf = Buffer.alloc(BUFFER_SIZE)
const bytes = new Uint8Array(buf.bytes)
stdin.name = 'count.stdin'
stdin.setup(buf, UV_TTY_MODE_NORMAL)
stdin.onEnd(() => stdin.close())
let braces = 0
let firstBrace = false
stdin.onRead(len => {
  try {
    let i = 0
    while (len--) {
      if (bytes[i] === 123) {
        braces++
        print(braces)
      } else if (bytes[i] === 125) {
        braces--
        print(braces)
        if (braces === 0) {
          print('done')
        }
      }
      i++
    }
  } catch (err) {
    print(err.message)
  }
})
stdin.resume()
