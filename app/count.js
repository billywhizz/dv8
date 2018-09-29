const { TTY } = module('tty', {})
const b = new Buffer()
b.alloc(64 * 1024)
let bytes = 0
const stdin = new TTY(0, len => bytes += len, () => {
    print(bytes)
    stdin.close()
}, () => {})
stdin.setup(b)
stdin.resume()
