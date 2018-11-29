const { createServer, TCP } = require('./lib/http.js')

const opts = {
  address: '0.0.0.0',
  type: TCP,
  port: 3000,
  secure: true,
  domains: ['dv8.billywhizz.io'],
  pipeline: false,
  parseHeaders: false
}

process.tt = setInterval(() => {
  const stats = process.stats()
  console.log(`queue: ${stats.queue}, ticks: ${stats.ticks}`)
}, 1000)

const server = createServer((req, res) => {
  const body = []
  req.onBody = (buf, len) => {
    body.push(buf.read(0, len))
  }
  req.onEnd = () => {
    res.statusCode = 200
    if (req.url === '/timer') {
      req.timer.start(res.end, 0)
      return
    }
    if (req.url === '/nextTick') return process.nextTick(res.end)
    res.end()
  }
}, opts)

const r = server.listen()
if (r !== 0) throw new Error('Listen Error')
