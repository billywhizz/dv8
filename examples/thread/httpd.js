function spawn () {
  const thread = process.spawn(() => {
    const server = require('../lib/http.js').createServer((req, res) => {
      res.setHeader('Content-Type', 'text/plain; charset=UTF-8')
      res.contentLength = 13
      res.end('Hello, World!')
    })
    server.listen()
    process.send({ id: process.TID })
    process.onMessage(m => {
      const command = JSON.parse(m.payload)
      if (command.shutdown) {
        server.sock.close()
        process.sock.close()
      }
    })
  }, result => {
    thread.sock.close()
    gc()
  }, { ipc: true })
  thread.onMessage(m => {
    const payload = JSON.parse(m.payload)
    payload.boot = Date.now() - thread.started
    print(JSON.stringify(payload))
    setTimeout(() => {
      thread.send({ shutdown: true })
    }, 5000)
  })
  thread.started = Date.now()
  return thread
}
let count = parseInt(process.args[2] || 1, 10)
function next () {
  spawn()
  if (--count) setTimeout(next, 1000)
}
next()
