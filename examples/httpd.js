function threadMain () {
  const { Socket, TCP } = module('socket', {})
  const { HTTPParser, REQUEST } = module('httpParser', {})

  const parsers = []

  const read = Buffer.alloc(64 * 1024)
  const write = Buffer.alloc(64 * 1024)
  const server = new Socket(TCP)
  const methods = { 0: 'DELETE', 1: 'GET', 2: 'HEAD', 3: 'POST', 4: 'PUT', 6: 'OPTIONS', 28: 'PATCH' }

  let resLength = write.write(`HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 13\r\n\r\nHello, World!`, 0)

  const createParser = () => {
    if (parsers.length) return parsers.shift()
    const work = Buffer.alloc(64 * 1024)
    const parser = new HTTPParser()
    const view = new DataView(work.bytes)
    const request = {
      method: 'GET',
      headers: '',
      major: 1,
      minor: 1,
      upgrade: false,
      keepalive: true
    }
    const bytes = new Uint8Array(work.bytes)
    parser.parse = skipHeaders => {
      request.major = bytes[0]
      request.minor = bytes[1]
      request.method = methods[bytes[2]]
      request.upgrade = bytes[3] === 1
      request.keepalive = bytes[4] === 1
      if (skipHeaders) {
        request.url = ''
        request.headers = ''
        return request
      }
      const urlLength = view.getUint16(5)
      const headerLength = view.getUint16(7)
      const str = work.read(16, 16 + urlLength + headerLength)
      request.url = str.substring(0, urlLength)
      request.headers = str.substring(urlLength, urlLength + headerLength)
      return request
    }
    parser.setup(read, work)
    return parser
  }

  function onConnect () {
    const client = new Socket(TCP)
    const parser = createParser()
    parser.onHeaders(() => {
      //const request = parser.parse()
      const r = client.write(resLength)
      if (r === resLength) return
      if (r < 0) return client.close()
      if (r < resLength) return client.pause()
    })
    client.setup(read, write)
    client.onClose(() => parsers.push(parser))
    client.onDrain(() => client.resume())
    client.onEnd(() => client.close())
    parser.reset(REQUEST, client)
    return client
  }

  server.onConnect(onConnect)
  server.listen('0.0.0.0', 3000)

  server.timer = setInterval(() => {
    resLength = write.write(`HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nServer: dv8\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 13\r\n\r\nHello, World!`, 0)
  }, 500)
}

let workers = parseInt(process.args[2] || '1')
while (workers--) {
  process.spawn(threadMain, result => print(`thread ${result.thread.id} done`))
}
