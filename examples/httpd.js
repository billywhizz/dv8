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

const request = {
  minorVersion: 1,
  bodyLength: 0,
  bodyBytes: 0,
  numHeaders: 0,
  headerSize: 0,
  path: '',
  method: '',
  headers: [[], [], [], [], [], [], [], []]
}

let methodLen = 0
let pathLen = 0
let numHeaders = 0
let ptrHeaders = BigInt(0)
let ptrPath = BigInt(0)
let ptrMethod = BigInt(0)
let nameLen = 0
let ptrName = BigInt(0)
let valLen = 0
let ptrVal = BigInt(0)

const BIG1 = BigInt(1)
const BIG2 = BigInt(2)
const BIG4 = BigInt(4)
const BIG8 = BigInt(8)
const BIG10 = BigInt(10)
const BIG12 = BigInt(12)
const BIG14 = BigInt(14)
const BIG15 = BigInt(15)
const BIG20 = BigInt(20)
const BIG23 = BigInt(23)
const BIG31 = BigInt(31)
const headers = request.headers
let i = 0

const nullHandler = address64 => rps++

function handler (address64) {
  request.minorVersion = Memory.readUint8(address64)
  methodLen = Memory.readUint8(address64 + BIG1)
  pathLen = Memory.readUint16(address64 + BIG2)
  request.bodyLength = Memory.readUint32(address64 + BIG4)
  request.bodyBytes = Memory.readUint32(address64 + BIG8)
  request.headerSize = Memory.readUint16(address64 + BIG12)
  numHeaders = Memory.readUint8(address64 + BIG14)
  ptrHeaders = Memory.readUint64(address64 + BIG15)
  ptrPath = Memory.readUint64(address64 + BIG23)
  ptrMethod = Memory.readUint64(address64 + BIG31)
  request.path = Memory.readString(ptrPath, BigInt(pathLen))
  request.method = Memory.readString(ptrMethod, BigInt(methodLen))
//  request.headers = Memory.readString(Memory.readUint64(ptrHeaders + BIG2), BigInt(request.headerSize))
  for (i = 0; i < numHeaders; i++) {
    nameLen = Memory.readUint16(ptrHeaders)
    ptrName = Memory.readUint64(ptrHeaders + BIG2)
    valLen = Memory.readUint16(ptrHeaders + BIG10)
    ptrVal = Memory.readUint64(ptrHeaders + BIG12)
    headers[i] = [Memory.readString(ptrName, BigInt(nameLen)), Memory.readString(ptrVal, BigInt(valLen))]
    ptrHeaders += BIG20
  }
  //dv8.print(stringify(request))
  rps++
}

server.onRequest(handler).listen('127.0.0.1', 3000)

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
