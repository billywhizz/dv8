const { Socket, TCP, UNIX } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})

const methods = { 0: 'DELETE', 1: 'GET', 2: 'HEAD', 3: 'POST', 4: 'PUT', 6: 'OPTIONS', 28: 'PATCH' }

const createServer = (callback, opts) => {
  opts = Object.assign({ address: '127.0.0.1', port: 3000, type: TCP, serverName: 'dv8' }, opts)
  const contexts = []
  const defaults = { 200: { message: 'OK' }, 400: { message: 'Bad Request' }, 404: { message: 'Not Found' }, 101: { message: 'Switching Protocols' } }
  const server = new Socket(opts.type)
  let _onConnect
  let _onDisconnect
  let _onClose
  const emptyFn = () => {}
  let now = (new Date()).toUTCString()
  const createContext = () => {
    if (contexts.length) return contexts.shift()
    const work = Buffer.alloc(1024 * 1024)
    const parser = new HTTPParser()
    const context = { in: Buffer.alloc(1024 * 1024), out: Buffer.alloc(1024 * 1024), work, parser, bytes: new Uint8Array(work.bytes), view: new DataView(work.bytes), client: null }
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
      const request = { major: 1, minor: 1, method: 1, upgrade: 0, keepalive: 1, url: '', headers: '', _onBody: emptyFn, _onEnd: emptyFn, onBody: fn => (request._onBody = fn), onEnd: fn => (request._onEnd = fn) }
      const response = {
        headers: [],
        version: '1.1',
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
            const { version, statusCode, contentLength } = response
            const page = defaults[statusCode] || defaults[404]
            let clientHeaders = ''
            if (response.headers.length) {
              clientHeaders = response.headers.join('')
            }
            payload = `HTTP/${version} ${statusCode} ${page.message}\r\nServer: ${opts.serverName}\r\nDate: ${now}\r\nContent-Length: ${contentLength}\r\n${clientHeaders}\r\n${chunk || ''}`
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
          if (client.closed) return
          let payload = chunk
          if (!response.headersSent) {
            const { statusCode, version } = response
            let { contentLength } = response
            if (contentLength === 0 && payload && payload.length) {
              contentLength = payload.length
            }
            const page = defaults[statusCode] || defaults[404]
            let clientHeaders = ''
            if (response.headers.length) {
              clientHeaders = response.headers.join('')
            }
            payload = `HTTP/${version} ${statusCode} ${page.message}\r\nServer: ${opts.serverName}\r\nDate: ${now}\r\nContent-Length: ${contentLength}\r\n${clientHeaders}\r\n${chunk || ''}`
          }
          if (!(payload && payload.length)) return
          // TODO: buffer overrun
          const len = out.write(payload, 0)
          const r = client.write(len)
          response.headersSent = true
          if (r === len) {
            if (request.keepalive) return
            if (client.queueSize() === 0) {
              client.close()
              return
            }
            client.onDrain(() => {
              client.close()
            })
            return
          }
          if (r < 0) {
            client.close()
          }
          if (r < len) {
            if (request.keepalive) return client.pause()
            if (client.queueSize() === 0) {
              client.close()
              return
            }
            client.onDrain(() => {
              client.close()
            })
          }
        }
      }
      request.major = bytes[0]
      request.minor = bytes[1]
      request.method = methods[bytes[2]]
      request.upgrade = bytes[3] === 1
      request.keepalive = bytes[4] === 1
      request.url = str.substring(0, urlLength)
      request.headers = str.substring(urlLength, urlLength + headerLength)
      context.response = response
      context.request = request
      callback(request, response, context)
    })
    parser.onRequest(() => context.request._onEnd())
    client.onClose(() => {
      contexts.push(context)
      client.closed = true
      if (_onDisconnect) {
        _onDisconnect(client, context)
      }
    })
    client.onEnd(() => {
      client.close()
    })
    client.onError((code, message) => {
      print('client.onError')
      print(code)
      print(message)
    })
    client.setup(context.in, context.out)
    if (_onConnect) {
      _onConnect(client, context)
    }
    parser.reset(REQUEST, client)
    return client
  })
  server.onClose(() => {
    clearTimeout(timer)
    if (_onClose) {
      _onClose()
    }
  })
  const { address, port } = opts
  return { now, contexts, sock: server, defaults, opts, listen: () => server.listen(address, port), onClose: fn => (_onClose = fn), onConnect: fn => (_onConnect = fn), onDisconnect: fn => (_onDisconnect = fn) }
}

module.exports = { TCP, UNIX, createServer }
