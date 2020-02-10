const { print, library, listHandles } = dv8
const { Socket } = library('net')

const buf = Buffer.alloc(16384)
const bytes64 = new BigUint64Array(buf.bytes)

let id = BigInt(0)

const server = new Socket()

server.onConnect(() => {
  print('server.onConnect')
  server.setup(buf, buf)
  server.write(8)
})

server.onData(len => {
  print('server.onData')
  bytes64[0] = id++
  server.write(len)
})

server.onEnd(() => {
  print('server.onEnd')
})

const client = new Socket()

client.onConnect(() => {
  print('client.onConnect')
  client.setup(buf, buf)
})

client.onData(len => {
  print('client.onData')
  print(bytes64[0].toString())
  setTimeout(() => client.write(len), 1000)
})

client.onEnd(() => {
  print('client.onEnd')
})

client.pair(server.pair())
