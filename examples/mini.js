const { print, memoryUsage } = dv8
const { Http } = dv8.library('net')
const server = new Http()
let rps = 0
let max = 0
server.onRequest((req, res) => {
  // 192k
  //const url = req.getUrl() // 180k
  //const method = req.getMethod() // 180k
  //const headers = req.getHeaders() // 150k
  // 3 above - 150k
  //const request = req.getRequest() // 120k
  //print(JSON.stringify(request, null, '  '))
  rps++
}).listen('127.0.0.1', 3000)
let then = dv8.hrtime()
const timer = setInterval(() => {
  const now = dv8.hrtime()
  const nanos = Number(now - then)
  const millis = nanos / 1000000
  const seconds = millis / 1000
  const rate = rps / seconds
  if (rps > max) max = rps
  dv8.print(`${rate.toFixed(0)} (\u001b[31m${max.toFixed(0)}\u001b[0m) ${memoryUsage().rss}`)
  rps = 0
  then = now
}, 1000)
