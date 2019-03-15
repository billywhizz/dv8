function threadMain () {
  const { Socket, TCP } = module('socket', {})
  const { PicoHTTPParser } = module('picoHttpParser', {})
  //const { HTTPParser, REQUEST } = module('httpParser', {})
  const { OS } = module('os', {})

  const SIGPIPE = 13
  const os = new OS()
  os.onSignal(signum => {
    return 0
  }, SIGPIPE)

  const parsers = []

  const createParser = () => {
    if (parsers.length) return parsers.shift()
    const work = Buffer.alloc(64 * 1024)
    const parser = new PicoHTTPParser()
    //const parser = new HTTPParser()
    parser.work = work
    parser.setup(read, work)
    return parser
  }

  function onConnect () {
    const client = new Socket(TCP)
    const parser = createParser()
    parser.onHeaders(() => client.write(r200len))
    client.setup(read, write)
    client.onClose(() => parsers.push(parser))
    client.onEnd(() => client.close())
    //parser.reset(REQUEST, client)
    parser.reset(client)
    return client
  }

  const r200 = `HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 13\r\n\r\nHello, World!`
  const r200len = r200.length
  const read = Buffer.alloc(64 * 1024)
  const write = Buffer.alloc(64 * 1024)
  const server = new Socket(TCP)

  write.write(r200, 0)
  server.onConnect(onConnect)
  server.listen('0.0.0.0', 3000)
  console.log(`${process.TID || process.PID} listening`)

  process.onExit = () => {
    console.log(`${process.TID || process.PID} exiting`)
  }
}

process.onExit = () => {
  print('process.exit')
}

let workers = parseInt(process.args[2] || '1')
while (workers--) {
  process.spawn(threadMain, result => {
    const { thread } = result
    print(`thread ${thread.id} done`)
  })
}
