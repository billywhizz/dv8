const { createServer } = require('http')

let timeouts = 0
let ticks = 0

const tt = setInterval(() => {
  console.log(`ticks: ${ticks}, timeouts: ${timeouts}`)
}, 1000)

createServer((req, res) => {
  if (req.url === '/timer') {
    setTimeout(() => res.end(), 0)
    timeouts++
    return
  }
  if (req.url === '/nextTick') {
    process.nextTick(() => res.end())
    ticks++
    return
  }
  res.end()
}).listen(3001)
