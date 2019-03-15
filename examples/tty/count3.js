const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const stdin = new TTY(0)
const BUFFER_SIZE = 256 * 1024
const buf = Buffer.alloc(BUFFER_SIZE)
stdin.setup(buf, UV_TTY_MODE_RAW)
let bytes = 0
let maxrss = 0
stdin.onRead(len => (bytes += len))
stdin.onEnd(() => {
  const elapsed = Date.now() - start
  const MiBRate = Math.floor(bytes / elapsed / 1000)
  console.log(`bytes: ${bytes}, time: ${elapsed}, rate: ${MiBRate}, maxrss: ${maxrss}`)
  stdin.close()
  clearTimeout(timer)
})
stdin.resume()
const start = Date.now()
const timer = setInterval(() => {
  const { rss } = process.memoryUsage()
  if (rss > maxrss) maxrss = rss
}, 1000)
