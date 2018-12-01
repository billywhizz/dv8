const { Timer } = module('timer', {})

const timers = []

function getTimer() {
  if (timers.length) return timers.shift()
  return new Timer()
}

const sleep = ms => new Promise(resolve => {
  const timer = getTimer()
  timer.start(() => {
    timers.push(timer)
    resolve()
  }, ms)
})

require('./lib/http.js').createServer(async (req, res) => {
  const payload = JSON.stringify(process.env)
  res.contentLength = payload.length
  res.setHeader('Content-Type', 'text/plain; charset=UTF-8')
  await sleep(200)
  res.writeString(payload.slice(0, 100))
  await sleep(400)
  res.writeString(payload.slice(100, 200))
  await sleep(400)
  res.writeString(payload.slice(200))
}).listen()
