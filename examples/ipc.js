const { print, library, memoryUsage, cpuUsage, pid } = dv8
const { Socket } = library('net')
const { Thread } = library('thread')

const spawn = (fnMain, fnComplete, fd, buf) => {
  const thread = new Thread()
  if (buf) {
    thread.start(fnMain, fnComplete, fd, buf)
  } else {
    thread.start(fnMain, fnComplete, fd)
  }
  return thread
}

function threadComplete (tid) {
  print(`thread ${tid} done`)
}

function printStats () {
  const rss = memoryUsage().rss
  const { user, system } = cpuUsage()
  const u = (user - last.user) / 1000000
  const s = (system - last.system) / 1000000
  last.user = user
  last.system = system
  const MB = last.bps / (1024 * 1024)
  const Mb = Math.floor(MB * 8)
  print(`main   Mbps ${Mb} rss ${rss} cpu ${u.toFixed(2)} / ${s.toFixed(2)}`)
  last.bps = 0
}

function threadMain () {
  const { print, library, tid, workerData } = dv8
  let buf = workerData
  let size = 0
  if (buf) {
    size = buf.size()
  } else {
    buf = Buffer.alloc(65536)
    size = buf.size
  }
  const { Socket } = library('net')
  const sock = new Socket()
  sock.onConnect(() => {
    print(`thread(${tid}).onConnect`)
    sock.setup(buf, buf)
    sock.write(size)
  })
  sock.onData(len => {
    sock.write(size)
  })
  sock.onEnd(() => {
    print(`thread(${tid}).onEnd`)
  })
  sock.pair(dv8.fd)
}

function createPair (stats, pid, shared = false) {
  const sock = new Socket()
  const buf = Buffer.alloc(65536, shared)
  sock.onConnect(() => {
    print(`process(${pid}).onConnect`)
    sock.setup(buf, buf)
  })
  sock.onData(len => {
    stats.bps += len
    sock.write(1)
  })
  sock.onEnd(() => {
    print(`process(${pid}).onEnd`)
  })
  if (shared) {
    spawn(threadMain, threadComplete, sock.pair(), buf)
  } else {
    spawn(threadMain, threadComplete, sock.pair())
  }
}

const last = { bps: 0, user: 0, system: 0 }

global.timer = setInterval(printStats, 1000)

createPair(last, pid(), false)
