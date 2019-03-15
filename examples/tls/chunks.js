const { Timer } = module('timer', {})
const { createServer, TCP } = require('./lib/http.js')

const timers = []
const getTimer = () => (timers.shift() || new Timer())
const sleep = (ms, timer) => new Promise(resolve => timer.start(() => resolve(), ms))

const startServer = options => {
  const server = createServer((req, res) => {
    req.onEnd(async () => {
      const timer = getTimer()
      const payload = JSON.stringify(process.env)
      res.contentLength = payload.length
      res.setHeader('Content-Type', 'text/plain; charset=UTF-8')
      await sleep(10, timer)
      res.writeString(payload.slice(0, 100))
      await sleep(10, timer)
      res.writeString(payload.slice(100, 200))
      await sleep(30, timer)
      res.writeString(payload.slice(200))
      timers.push(timer)
    })
  }, options)

  const ok = server.listen()
  if (ok !== 0) throw new Error('Listen Failed')
  // const { now, sock, defaults, opts, contexts } = server  
}

startServer({
  type: TCP,
  serverName: 'foo',
  address: '0.0.0.0',
  port: 3000
})
