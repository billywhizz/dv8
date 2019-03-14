const { UV_TTY_MODE_RAW, TTY } = module('tty', {})
const stdin = new TTY(0)
const BUFFER_SIZE = 256 * 1024
const buf = Buffer.alloc(BUFFER_SIZE)
let maxrss = 0
stdin.setup(buf, UV_TTY_MODE_RAW)
stdin.onEnd(() => {
  const elapsed = Date.now() - start
  const stats = new BigUint64Array(20)
  stdin.stats(stats)
  const [, , bytes] = stats
  const MiBRate = bytes / BigInt(elapsed) / BigInt(1000)
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
