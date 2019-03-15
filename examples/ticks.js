require('./lib/http.js', { address: '0.0.0.0' }).createServer((req, res) => {
  print('hello')
  res.setHeader('Content-Type', 'text/plain; charset=UTF-8')
  res.contentLength = 13
  res.end('Hello, World!')
}).listen()
