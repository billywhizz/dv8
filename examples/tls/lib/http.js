const { Socket, TCP, UNIX } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const contexts = []

let serviceName = process.env.SERVICE_NAME || 'dv8'

const createContext = (responses) => {
  if (contexts.length) return contexts.shift()
  const work = Buffer.alloc(16384)
  const parser = new HTTPParser()
  const context = { in: Buffer.alloc(16384), out: Buffer.alloc(16384), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), request: { major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', body: [] }, client: null }
  const { out, request } = context
  const response = {
    statusCode: 200,
    end: () => {
      const { client } = context
      const len = out.write(responses[response.statusCode] || responses['404'], 0)
      const r = client.write(len)
      if (r === len) {
        if (request.keepalive === 1) return
        if (client.queueSize() === 0) {
          return client.close()
        }
        client.onDrain(() => {
          client.close()
        })
        return
      }
      if (r < 0) client.close()
      if (r < len) {
        if (request.keepalive === 1) return client.pause()
        if (client.queueSize() === 0) {
          return client.close()
        }
        client.onDrain(() => client.close())
      }
    }
  }
  context.response = response
  parser.setup(context.in, work)
  return context
}

const createServer = (callback, opts = { type: TCP }) => {
  const responses = {
    200: `HTTP/1.1 200 OK\r\nServer: ${serviceName}\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 0\r\n\r\n`,
    404: `HTTP/1.1 404 Not Found\r\nServer: ${serviceName}\r\nDate: ${(new Date()).toUTCString()}\r\nContent-Length: 0\r\n\r\n`
  }
  const timer = setInterval(() => {
    const now = new Date().toUTCString()
    for (const key of Object.keys(responses)) {
      responses[key] = responses[key].replace(/Date: .+\r\n/, `Date: ${now}\r\n`)
    }
  }, 1000)
  const server = new Socket(opts.type)
  server.onConnect(fd => {
    const client = new Socket(opts.type)
    const context = createContext(responses)
    const { bytes, view, work, parser, request, response } = context
    const { body } = request
    context.client = client
    parser.onBody(len => body.push(work.read(0, len)))
    parser.onHeaders(() => {
      if (body.length) body.length = 0
      const urlLength = view.getUint16(5)
      const headerLength = view.getUint16(7)
      const str = work.read(16, 16 + urlLength + headerLength)
      request.major = bytes[0]
      request.minor = bytes[1]
      request.method = bytes[2]
      request.upgrade = bytes[3]
      request.keepalive = bytes[4]
      request.url = str.substring(0, urlLength)
      request.headers = str.substring(urlLength, urlLength + headerLength)
    })
    parser.onRequest(() => callback(request, response))
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onClose(() => contexts.push(context))
    client.onEnd(() => client.close())
    client.setNoDelay(true)
    client.setKeepAlive(true, 3000)
    parser.reset(REQUEST, client)
  })
  server.onClose(() => {
    clearTimeout(timer)
  })
  const { address = '127.0.0.1', port = 3000 } = opts
  return {
    listen: () => {
      return server.listen(address, port)
    }
  }
}

module.exports = { TCP, UNIX, createServer }
