require('./lib/http.js').createServer((req, res) => {
  res.setHeader('Content-Type', 'text/plain; charset=UTF-8')
  res.end('Hello, World!')
}).listen()
