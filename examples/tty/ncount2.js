const { TTY } = process.binding('tty_wrap')
let bytes = 0
let maxrss = 0
const tty = new TTY(0, true)
tty.setRawMode(true)
tty.onread = (nread, buf) => {
  if (nread < 1) {
    const elapsed = Date.now() - start
    const MiBRate = Math.floor(bytes / elapsed / 1000)
    console.log(`bytes: ${bytes}, time: ${elapsed}, rate: ${MiBRate}, maxrss: ${maxrss}`)
    clearTimeout(timer)
    return
  }
  bytes += nread
}
tty.readStart()
const start = Date.now()
const timer = setInterval(() => {
  const { rss } = process.memoryUsage()
  if (rss > maxrss) maxrss = rss
}, 1000)
