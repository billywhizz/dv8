const { Socket, UNIX } = module('socket', {})
const SIZE = parseInt(process.args[2] || '16384')

const [rb, wb] = [Buffer.alloc(SIZE), Buffer.alloc(SIZE)]
const stats = new BigUint64Array(20)

function dumpStats (name) {
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

server.onConnect(() => {
  server.setup(rb, wb)
  server.resume()
})

server.onRead(len => {
  server.write(SIZE)
})

server.onClose(() => {
  server.stats(stats)
  dumpStats('server')
})

const fd = server.open()

const client = new Socket(UNIX)

client.onConnect(() => {
  client.setup(wb, rb)
  client.write(SIZE)
  client.resume()
})

client.onRead(len => {
  client.write(SIZE)
})

client.onClose(() => {
  client.stats(stats)
  dumpStats('client')
})

client.open(fd)

function toMib (bytes) {
  return (((bytes * 8n) / (1024n * 1024n)) * 100n) / 100n
}

const last = {
  clientRead: 0n, clientWrite: 0n, serverRead: 0n, serverWrite: 0n
}

setInterval(() => {
  client.stats(stats)
  const clientRead = stats[2]
  const clientWrite = stats[10]
  dumpStats('client')
  server.stats(stats)
  const serverRead = stats[2]
  const serverWrite = stats[10]
  dumpStats('server')
  print(`client.read: ${toMib(clientRead - last.clientRead)}
client.write: ${toMib(clientWrite - last.clientWrite)}
server.read: ${toMib(serverRead - last.serverRead)}
server.write: ${toMib(serverWrite - last.serverWrite)}
`)
  last.clientRead = clientRead
  last.clientWrite = clientWrite
  last.serverRead = serverRead
  last.serverWrite = serverWrite
}, 1000)
