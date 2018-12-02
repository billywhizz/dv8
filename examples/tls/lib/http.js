const { Socket, TCP, UNIX } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const createServer = (callback, opts = { type: TCP, serverName: 'dv8' }) => {
  const contexts = []
  const defaults = { 200: { message: 'OK' }, 400: { message: 'Bad Request' }, 404: { message: 'Not Found' } }
  const server = new Socket(opts.type)
  let _onConnect
  let _onDisconnect
  let _onClose
  let now = (new Date()).toUTCString()
  const createContext = () => {
    if (contexts.length) return contexts.shift()
    const work = Buffer.alloc(16384)
    const parser = new HTTPParser()
    const context = { in: Buffer.alloc(16384), out: Buffer.alloc(16384), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), client: null }
    parser.setup(context.in, work)
    return context
  }
  const timer = setInterval(() => {
    now = (new Date()).toUTCString()
  }, 1000)
  server.onConnect(() => {
    const client = new Socket(opts.type)
    client.closed = false
    const context = createContext()
    const { bytes, view, work, parser, out } = context
    context.client = client
    parser.onBody(len => context.request._onBody(work, len))
    parser.onHeaders(() => {
      const urlLength = view.getUint16(5)
      const headerLength = view.getUint16(7)
      const str = work.read(16, 16 + urlLength + headerLength)
      const request = { major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', onBody: fn => (request._onBody = fn), onEnd: fn => (request._onEnd = fn) }
      const response = {
        headers: [],
        headersSent: false,
        setHeader: (k, v) => {
          response.headers.push(`${k}: ${v}\r\n`)
        },
        statusCode: 200,
        buffer: out,
        contentLength: 0,
        writeString: chunk => {
          const { client } = context
          if (client.closed) return
          let payload = chunk
          if (!response.headersSent) {
            const { statusCode, contentLength } = response
            const page = defaults[statusCode] || defaults[404]
            let clientHeaders = ''
            if (response.headers.length) {
              clientHeaders = response.headers.join('')
            }
            payload = `HTTP/1.1 ${statusCode} ${page.message}\r\nServer: ${opts.serverName}\r\nDate: ${now}\r\nContent-Length: ${contentLength}\r\n${clientHeaders}\r\n${chunk || ''}`
          }
          // TODO: buffer overrun
          const len = out.write(payload, 0)
          const r = client.write(len)
          response.headersSent = true
          if (r === len) return
          if (r < 0) client.close()
          if (r < len) return client.pause()
        },
        end: chunk => {
          const { client } = context
          let payload = chunk
          if (!response.headersSent) {
            const { statusCode } = response
            let { contentLength } = response
            if (contentLength === 0 && payload && payload.length) {
              contentLength = payload.length
            }
            const page = defaults[statusCode] || defaults[404]
            let clientHeaders = ''
            if (response.headers.length) {
              clientHeaders = response.headers.join('')
            }
            payload = `HTTP/1.1 ${statusCode} ${page.message}\r\nServer: ${opts.serverName}\r\nDate: ${now}\r\nContent-Length: ${contentLength}\r\n${clientHeaders}\r\n${chunk || ''}`
          }
          if (!(payload && payload.length)) return
          // TODO: buffer overrun
          const len = out.write(payload, 0)
          const r = client.write(len)
          response.headersSent = true
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
    parser.onRequest(() => context.request._onEnd())
    client.onClose(() => {
      contexts.push(context)
      client.closed = true
      if (_onDisconnect) {
        _onDisconnect(client, context)
      }
    })
    client.onEnd(() => client.close())
    client.onError((code, message) => {
      print(code)
      print(message)
    })
    client.setup(context.in, context.out)
    parser.reset(REQUEST, client)
    if (_onConnect) {
      _onConnect(client, context)
    }
    return client
  })
  server.onClose(() => {
    clearTimeout(timer)
    if (_onClose) {
      _onClose()
    }
  })
  const { address = '127.0.0.1', port = 3000 } = opts
  return { now, contexts, sock: server, defaults, opts, listen: () => server.listen(address, port), onClose: fn => (_onClose = fn), onConnect: fn => (_onConnect = fn), onDisconnect: fn => (_onDisconnect = fn) }
}

module.exports = { TCP, UNIX, createServer }
