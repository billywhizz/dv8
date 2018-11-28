const { Socket, UNIX } = module('socket', {})
const SIZE = parseInt(process.args[2] || '16384')
const [rb, wb] = [Buffer.alloc(SIZE), Buffer.alloc(SIZE)]

const stats = new BigUint64Array(20)

function dumpStats(name) {
  print(`${name}
close            : ${stats[0]}
error            : ${stats[1]}
in.read          : ${stats[2]}
in.pause         : ${stats[3]}
in.data          : ${stats[4]}
in.resume        : ${stats[5]}
in.end           : ${stats[6]}
out.write        : ${stats[10]}
out.incomplete   : ${stats[11]}
out.full         : ${stats[12]}
out.drain        : ${stats[13]}
out.maxQueue     : ${stats[14]}
out.alloc        : ${stats[15]}
out.free         : ${stats[16]}
out.eagain       : ${stats[17]}
`)
}

const server = new Socket(UNIX)

server.onConnect(fd => {
  server.setup(fd, rb, wb)
})

server.onClose(() => {
  server.stats(stats)
  dumpStats('server')
})

const fd = server.open()

const thread = process.spawn(() => {
  const { Socket, UNIX } = module('socket', {})
  const SIZE = parseInt(process.args[2] || '16384')
  const [rb, wb] = [Buffer.alloc(SIZE), Buffer.alloc(SIZE)]
  const stats = new BigUint64Array(20)
  function dumpStats(name) {
    print(`${name}
close            : ${stats[0]}
error            : ${stats[1]}
in.read          : ${stats[2]}
in.pause         : ${stats[3]}
in.data          : ${stats[4]}
in.resume        : ${stats[5]}
in.end           : ${stats[6]}
out.write        : ${stats[10]}
out.incomplete   : ${stats[11]}
out.full         : ${stats[12]}
out.drain        : ${stats[13]}
out.maxQueue     : ${stats[14]}
out.alloc        : ${stats[15]}
out.free         : ${stats[16]}
out.eagain       : ${stats[17]}
  `)
  }
  
  const client = new Socket(UNIX)

  function next() {
    client.write(SIZE)
    process.nextTick(next)
  }

  client.onConnect(fd => {
    client.setup(fd, wb, rb)
    next()
  })

  client.onClose(() => {
    client.stats(stats)
    dumpStats('server')
  })

  const dv = new DataView(global.workerData.bytes)
  const fd = dv.getUint32(1000)
  client.open(fd)
}, result => {
  print('thread says goodbye')
})

thread.view.setUint32(1000, fd)

let then = Date.now()

function toMib(bytes) {
  return (((bytes * 8n) / (1024n * 1024n)) * 100n) / 100n
}

const last = {
  serverRead: 0n, serverWrite: 0n
}

const timer = setInterval(() => {
  const now = Date.now()
  const elapsed = now - then
  server.stats(stats)
  const serverRead = stats[2]
  console.log(serverRead - last.serverRead)
  const serverWrite = stats[10]
  //dumpStats('server')
  print(`server.read: ${toMib(serverRead - last.serverRead)}
server.write: ${toMib(serverWrite - last.serverWrite)}
`)
  last.serverRead = serverRead
  last.serverWrite = serverWrite
  then = Date.now()
}, 1000)
