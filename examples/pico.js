const { Socket, TCP } = module('socket', {})
const { PicoHTTPParser, REQUEST } = module('picoHttpParser', {})

const server = new Socket(TCP)

const WORK_SIZE = 1024
const READ_SIZE = 1024
const WRITE_SIZE = 1024

const r200 = `HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 13\r\n\r\nHello, World!`
const r200len = r200.length

server.onConnect(() => {
  const client = new Socket(TCP)
  const parser = new PicoHTTPParser()
  const buffers = {
    work: Buffer.alloc(WORK_SIZE),
    read: Buffer.alloc(READ_SIZE),
    write: Buffer.alloc(WRITE_SIZE)
  }
  buffers.write.write(r200, 0)
  parser.onHeaders(() => client.write(r200len))
  client.setup(buffers.read, buffers.write)
  parser.setup(buffers.read, buffers.work)
  parser.reset(client)
  return client
})

server.listen('0.0.0.0', 3000)
