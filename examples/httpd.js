const { print, library, memoryUsage, cpuUsage } = dv8
const stats = {
  rps: 0, rss: 0, user: 0, system: 0
}
const lastUsage = { user: 0, system: 0 }
let rps = 0

const { Http } = library('net')
const server = new Http()
server.onRequest(() => rps++).listen('127.0.0.1', 3000)

function dumpStats () {
  const { rps, rss, user, system } = stats
  print(`mem ${rss} rate ${rps} cpu ${user.toFixed(2)} / ${system.toFixed(2)}`)
}

function onTimer () {
  stats.rss = memoryUsage().rss
  const { user, system } = cpuUsage()
  stats.user = (user - lastUsage.user) / 1000000
  stats.system = (system - lastUsage.system) / 1000000
  stats.rps = rps
  lastUsage.user = user
  lastUsage.system = system
  rps = 0
  dumpStats()
}

server.timer = setInterval(onTimer, 1000)
