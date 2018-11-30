const { Socket, TCP, UNIX } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const contexts = []
const defaults = { 200: { message: 'OK' }, 404: { message: 'Not Found' } }

const createContext = () => {
  if (contexts.length) return contexts.shift()
  const work = Buffer.alloc(16384)
  const parser = new HTTPParser()
  const context = { in: Buffer.alloc(16384), out: Buffer.alloc(16384), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), client: null }
  parser.setup(context.in, work)
  return context
}

function endHandler (context) {
  const { client, request, response, out } = context
  const { statusCode } = response
  const payload = defaults[statusCode] || defaults[404]
  const len = out.write(`HTTP/1.1 ${statusCode} ${payload.message}\r\nServer: dv8\r\nDate: ${now}\r\nContent-Length: 0\r\n\r\n`, 0)
  const r = client.write(len)
  if (r === len) {
    if (request.keepalive === 1) return
    if (client.queueSize() === 0) return client.close()
    client.onDrain(() => client.close())
    return
  }
  if (r < 0) return client.close()
  if (r < len) {
    if (request.keepalive === 1) return client.pause()
    if (client.queueSize() === 0) return client.close()
    client.onDrain(() => client.close())
  }
}

function onConnect (fd, opts, callback) {
  const client = new Socket(opts.type)
  const context = createContext()
  const { bytes, view, work, parser } = context
  context.client = client
  parser.onBody(len => context.request.onBody(work, len))
  parser.onHeaders(() => {
    const urlLength = view.getUint16(5)
    const headerLength = view.getUint16(7)
    const str = work.read(16, 16 + urlLength + headerLength)
    const request = { timer: context.timer, major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', onBody: () => {}, onEnd: () => {} }
    const response = { statusCode: 200, end: () => endHandler(context) }
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
  client.onClose(() => contexts.push(context))
  client.onEnd(() => client.close())
  parser.reset(REQUEST, client)
}

const createServer = (callback, opts = { type: TCP }) => {
  const server = new Socket(opts.type)
  server.onConnect(fd => onConnect(fd, opts, callback))
  server.onClose(() => clearTimeout(timer))
  const { address = '127.0.0.1', port = 3000 } = opts
  return { listen: () => server.listen(address, port) }
}

let now = (new Date()).toUTCString()
const timer = setInterval(() => {
  now = (new Date()).toUTCString()
}, 1000)
timer.unref()

module.exports = { TCP, UNIX, createServer }
