const { Socket, UNIX } = module('socket', {})
const [rb, wb] = [Buffer.alloc(16384), Buffer.alloc(16384)]

const server = new Socket(UNIX)
server.onConnect(fd => {
  print(`server.onConnect: ${fd}`)
  server.setup(fd, rb, wb)
})
server.onRead(len => {
  print(`server.onRead:\n${rb.read(0, len)}`)
  server.write(rb.write('pong'))
})
const fd = server.open()

const client = new Socket(UNIX)
client.onConnect(fd => {
  print(`client.onConnect: ${fd}`)
  client.setup(fd, wb, rb)
  client.write(rb.write('ping'))
})
client.onRead(len => {
  print(`client.onRead:\n${rb.read(0, len)}`)
})
client.open(fd)
