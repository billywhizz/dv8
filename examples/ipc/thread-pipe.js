
function createServer () {
  const BUFFER_SIZE = 256 * 1024
  const { Socket, UNIX } = module('socket', {})
  const buf = Buffer.alloc(BUFFER_SIZE)

  const server = new Socket(UNIX)
  const stats = { in: 0, out: 0 }

  server.onConnect(() => {
    server.setup(buf, buf)
    server.resume()
  })

  server.onRead(len => {
    stats.in += len
    server.write(len)
    stats.out += len
  })

  const timer = setInterval(() => {
    stats.in = Math.floor((stats.in / (1024 * 1024)) * 100) / 100
    stats.out = Math.floor((stats.out / (1024 * 1024)) * 100) / 100
    print(`server: ${JSON.stringify(stats, null, '  ')}`)
    stats.in = stats.out = 0
  }, 1000)

  process.send({ fd: server.open() })
}

function createClient () {
  const BUFFER_SIZE = 256 * 1024
  const { Socket, UNIX } = module('socket', {})
  const buf = Buffer.alloc(BUFFER_SIZE)

  const client = new Socket(UNIX)
  const stats = { in: 0, out: 0 }

  client.onConnect(() => {
    client.setup(buf, buf)
    client.resume()
    function next() {
      client.write(BUFFER_SIZE)
      stats.out += BUFFER_SIZE
      process.nextTick(next)
    }
    client.write(BUFFER_SIZE)
    next()
  })

  client.onRead(len => {
    stats.in += len
    client.write(len)
    stats.out += len
  })

  const timer = setInterval(() => {
    stats.in = Math.floor((stats.in / (1024 * 1024)) * 100) / 100
    stats.out = Math.floor((stats.out / (1024 * 1024)) * 100) / 100
    print(`client: ${JSON.stringify(stats, null, '  ')}`)
    stats.in = stats.out = 0
  }, 1000)

  process.onMessage(message => {
    const { payload } = message
    const { fd } = JSON.parse(payload)
    print(`fd: ${fd}`)
    client.open(fd)
  })
}

const serverThread = process.spawn(createServer, result => {}, { ipc: true })
const clientThread = process.spawn(createClient, result => {}, { ipc: true })
// forward messages from server to client
serverThread.onMessage(message => clientThread.send(JSON.parse(message.payload)))
