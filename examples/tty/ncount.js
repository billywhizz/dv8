let bytes = 0
let maxrss = 0
process.stdin.on('data', chunk => {
  bytes += chunk.length
})
process.stdin.on('end', () => {
  const elapsed = Date.now() - start
  const MiBRate = Math.floor(bytes / elapsed / 1000)
  console.log(`bytes: ${bytes}, time: ${elapsed}, rate: ${MiBRate}, maxrss: ${maxrss}`)
  clearTimeout(timer)
})
const start = Date.now()
const timer = setInterval(() => {
  const { rss } = process.memoryUsage()
  if (rss > maxrss) maxrss = rss
}, 1000)
