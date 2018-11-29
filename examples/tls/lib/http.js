const { Socket, TCP, UNIX } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})
const { Timer } = module('timer', {})

const contexts = []

let serviceName = process.env.SERVICE_NAME || 'dv8'
let MAX_PIPELINE = parseInt(process.env.MAX_PIPELINE || '5', 10)

const foo = () => {}

const createContext = (responses) => {
  if (contexts.length) return contexts.shift()
  const work = Buffer.alloc(16384)
  const parser = new HTTPParser()
  const context = { timer: new Timer(), queue: [], in: Buffer.alloc(16384), out: Buffer.alloc(16384), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), client: null }
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
    const { bytes, view, work, parser, out } = context
    context.client = client
    let inflight = 0
    let paused = false
    parser.onBody(len => context.request.onBody(work, len))
    parser.onHeaders(() => {
      inflight++
      if (inflight > MAX_PIPELINE && !paused) {
        print(`pausing: ${inflight}`)
        parser.pause()
        client.pause()
        paused = true
      }
      const urlLength = view.getUint16(5)
      const headerLength = view.getUint16(7)
      const str = work.read(16, 16 + urlLength + headerLength)
      const request = { timer: context.timer, major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', onBody: foo, onEnd: foo }
      const response = {
        statusCode: 200,
        end: () => {
          const { client } = context
          inflight--
          if (inflight < MAX_PIPELINE && paused) {
            print(`resuming: ${inflight}`)
            parser.resume()
            client.resume()
            paused = false
          }
          const len = out.write(responses[response.statusCode] || responses['404'], 0)
          const r = client.write(len)
          if (r === len) {
            if (request.keepalive === 1) return
            if (client.queueSize() === 0) return client.close()
            client.onDrain(() => client.close())
            return
          }
          if (r < 0) client.close()
          if (r < len) {
            if (request.keepalive === 1) return client.pause()
            if (client.queueSize() === 0) return client.close()
            client.onDrain(() => client.close())
          }
        }
      }
      request.major = bytes[0]
      request.minor = bytes[1]
      request.method = bytes[2]
      request.upgrade = bytes[3]
      request.keepalive = bytes[4]
      request.url = str.substring(0, urlLength)
      request.headers = str.substring(urlLength, urlLength + headerLength)
      context.response = response
      context.request = request
      callback(request, response)
    })
    parser.onRequest(() => context.request.onEnd())
    client.setup(fd, context.in, context.out)
    client.address = client.remoteAddress()
    client.onClose(() => {
      context.queue.length = 0
      contexts.push(context)
    })
    client.onEnd(() => client.close())
    //client.setNoDelay(true)
    //client.setKeepAlive(true, 3000)
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
