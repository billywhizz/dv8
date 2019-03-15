const { Socket, UNIX } = module('socket', {})
const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]

const server = new Socket(UNIX)

server.onConnect(() => {
  print(`server.onConnect`)
  server.setup(rb, wb)
  server.resume()
})

server.onRead(len => {
  print(`server.onRead:\n${rb.read(0, len)}`)
  const r = server.write(wb.write('pong'))
  print(`server.write: ${r}`)
})

const fd = server.open()

const client = new Socket(UNIX)

client.onConnect(() => {
  print(`client.onConnect`)
  client.setup(wb, rb)
  client.resume()
  const r = client.write(rb.write('ping'))
  print(`client.write: ${r}`)
})

client.onRead(len => {
  print(`client.onRead:\n${wb.read(0, len)}`)
  client.close()
})

client.open(fd)
