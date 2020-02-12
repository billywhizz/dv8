const { print, library, memoryUsage, cpuUsage } = dv8
const stats = {
  rps: 0, rss: 0, user: 0, system: 0
}
const lastUsage = { user: 0, system: 0 }
let rps = 0

const { Http } = library('net')
const server = new Http()

const { Memory } = library('memory')

const stringify = (o, sp = '  ') => JSON.stringify(o, (k, v) => (typeof v === 'bigint') ? v.toString() : v, sp)

server.onRequest((address64, size) => {
  const minorVersion = Memory.readUint32(address64)
  const state = Memory.readUint32(address64 + BigInt(4))
  const methodLen = Memory.readUint64(address64 + BigInt(8))
  const pathLen = Memory.readUint64(address64 + BigInt(16))
  const bodyLength = Number(Memory.readUint64(address64 + BigInt(24)))
  const bodyBytes = Number(Memory.readUint64(address64 + BigInt(32)))
  const headerSize = Number(Memory.readUint64(address64 + BigInt(40)))
  const numHeaders = Number(Memory.readUint64(address64 + BigInt(48)))
  const ptrHeaders = Memory.readUint64(address64 + BigInt(56))
  const ptrPath = Memory.readUint64(address64 + BigInt(64))
  const ptrMethod = Memory.readUint64(address64 + BigInt(72))
  const path = Memory.readString(ptrPath, pathLen)
  const method = Memory.readString(ptrMethod, methodLen)
  let off = 0
  //const headers = {}
  for (let i = 0; i < numHeaders; i++) {
    const nameLen = Memory.readUint64(ptrHeaders + BigInt(off))
    const ptrName = Memory.readUint64(ptrHeaders + BigInt(off + 8))
    const name = Memory.readString(ptrName, nameLen)
    const valLen = Memory.readUint64(ptrHeaders + BigInt(off + 16))
    const ptrVal = Memory.readUint64(ptrHeaders + BigInt(off + 24))
    const val = Memory.readString(ptrVal, valLen)
    //headers[name] = val
    off += 32
  }
/*
  print(stringify({
    minorVersion,
    bodyLength,
    bodyBytes,
    headerSize,
    numHeaders,
    path,
    method,
    headers
  }))
*/
  rps++
}).listen('127.0.0.1', 3000)

function dumpStats () {
  const { rps, rss, user, system } = stats
  print(`mem ${rss} rate ${rps} cpu ${user.toFixed(2)} / ${system.toFixed(2)}`)
}

function onTimer () {
  try {
    stats.rss = memoryUsage().rss
    const { user, system } = cpuUsage()
    stats.user = (user - lastUsage.user) / 1000000
    stats.system = (system - lastUsage.system) / 1000000
    stats.rps = rps
    lastUsage.user = user
    lastUsage.system = system
    rps = 0
    dumpStats()
  } catch (err) {
    print(err.stack)
  }
}

server.timer = setInterval(onTimer, 1000)
