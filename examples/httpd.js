require('./lib/debugger.js').start()
require('./lib/http.js').createServer((req, res) => {
  res.setHeader('Content-Type', 'text/plain; charset=UTF-8')
  res.contentLength = 13
  res.end('Hello, World!')
}, { address: '0.0.0.0' }).listen()
