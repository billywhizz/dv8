function threadMain () {
  const { Socket, TCP } = module('socket', {})
  const read = Buffer.alloc(64 * 1024)
  const r200len = read.write(`HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 13\r\n\r\nHello, World!`, 0)
  const write = Buffer.alloc(r200len)

  let requestSize = 0
  read.copy(write, r200len)

  const server = new Socket(TCP)

  server.onConnect(() => {
    const client = new Socket(TCP)
    let off = 0
    client.setup(read, write)
    client.onEnd(() => client.close())
    client.onRead(len => {
      if (requestSize === 0) {
        const str = read.read(0, len)
        requestSize = str.indexOf('\r\n\r\n') + 4
      }
      let numRequests = (off + len) / requestSize
      off = (off + len) % requestSize
      numRequests = Math.floor(numRequests)
      while (numRequests--) {
        client.write(r200len)
      }
    })
    return client
  })

  server.listen('0.0.0.0', 3000)
}

const createWorker = () => process.spawn(threadMain, result => {}, { ipc: false })

createWorker()
createWorker()
createWorker()
createWorker()
