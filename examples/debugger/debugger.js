const { hexy } = require('./hexy.js')
const { TCP, Socket } = module('socket', {})
const { HTTPParser, REQUEST } = module('httpParser', {})
const { WSParser, createMessage } = require('./websocket.js')

const buf = Buffer.alloc(16384)
const work = Buffer.alloc(16384)
const bytes = new Uint8Array(buf.bytes)
const view = new DataView(work.bytes)

const methods = { 0: 'DELETE', 1: 'GET', 2: 'HEAD', 3: 'POST', 4: 'PUT', 6: 'OPTIONS', 28: 'PATCH' }

const sock = new Socket(TCP)

sock.onConnect(() => {
  const client = new Socket(TCP)
  const http = new HTTPParser()
  http.setup(buf, work)
  http.onHeaders(() => {
    const urlLength = view.getUint16(5)
    const headerLength = view.getUint16(7)
    const str = work.read(16, 16 + urlLength + headerLength)
    const url = str.substring(0, urlLength)
    const method = methods(bytes[2])
    const upgrade = bytes[3] === 1
    const headers = str.substring(urlLength, urlLength + headerLength)
    print(url)
    print(headers)
    if (upgrade) {
      const websocket = new WSParser()
      websocket.onStart(() => {
        client.upgrade = true
      })
      websocket.onMessage(message => {
  
      })
      websocket.onEnd(() => {
  
      })
      websocket.upgrade(client, url, headers)
      client.websocket = websocket
    } else {
      switch (true) {
        case (method === 'GET' && url === ''):

          break;
        default:
          
      }
    }
/*
    if ( request.headers['sec-websocket-version'] ) {
      request.version = request.headers['sec-websocket-version'][0]
     } else {
      return false
     }
     request.version = request.headers['sec-websocket-version'][0]
     var wskey = request.headers['sec-websocket-key'][0]
     var cookie = ''
     if(request.headers.hasOwnProperty('cookie')) {
       cookie = request.headers.cookie[0]
     }
     var shasum = new Hash('sha1')
     shasum.update(wskey)
     shasum.update('258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
     var res = 'HTTP/1.1 101 Switching Protocols\r\n'
       + 'Upgrade: websocket\r\n'
       + 'Connection: Upgrade\r\n'
       + 'Set-Cookie: ' + cookie + '\r\n'
       + 'Sec-WebSocket-Accept: ' + shasum.digest('base64') + '\r\n'
     if(request.headers.hasOwnProperty('sec-websocket-protocol')) {
       request.protocol = request.headers['sec-websocket-protocol'][0]
       res += 'Sec-Websocket-Protocol: ' + request.protocol + '\r\n'
     }
     res += '\r\n'
*/
  })
  client.onClose(() => {})
  client.onEnd(() => client.close())
  client.onError((code, message) => {
    print(code)
    print(message)
  })
  client.onRead(len => {
    print(buf.read(0, len))
    print(hexy(bytes.slice(0, len)))
    if (client.upgrade) client.websocket.execute(len)
  })
  client.setup(buf, buf)
  http.reset(REQUEST, client)
  return client
})

sock.listen('127.0.0.1', 3000)
