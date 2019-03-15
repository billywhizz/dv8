function threadMain () {
  const { Socket, TCP } = module('socket', {})
  const { HTTPParser, REQUEST } = module('httpParser', {})

  const createParser = () => {
    const work = Buffer.alloc(1024)
    const parser = new HTTPParser()
    parser.setup(read, work)
    return parser
  }

  function onConnect () {
    const client = new Socket(TCP)
    const parser = createParser()
    parser.onRequest(() => client.write(r200len))
    client.setup(read, write)
    client.onEnd(() => client.close())
    parser.reset(REQUEST, client)
    return client
  }

  const r200 = `HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 13\r\n\r\nHello, World!`
  const r200len = r200.length
  const read = Buffer.alloc(64 * 1024)
  const write = Buffer.alloc(r200len)
  const server = new Socket(TCP)

  write.write(r200, 0)
  server.onConnect(onConnect)
  server.listen('0.0.0.0', 3000)
  console.log(`${process.TID || process.PID} listening`)
}

let workers = parseInt(process.args[2] || '1')
while (workers--) process.spawn(threadMain, result => {})
