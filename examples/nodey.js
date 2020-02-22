let rps = 0

dv8.library('net').Http.createServer((req, res) => {
  rps++
  const url = req.getUrl()
  if (url === '/') {
    return
  }
  const method = req.getMethod()
  if (method === 'POST') {
    const headers = req.getHeaders()
  }
}).listen('127.0.0.1', 3000)

setInterval(() => {
  dv8.print(`${rps} ${dv8.memoryUsage().rss}`)
  rps = 0
}, 1000)
