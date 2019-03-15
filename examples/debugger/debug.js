function threadFunc () {
  const { createServer } = require('../lib/http.js')
  const { WSParser, createMessage } = require('../lib/websocket.js')
  const { Hash } = module('openssl', {})
  const { buf2b64 } = require('../lib/util.js')

  const [ wb, rb ] = [ Buffer.alloc(1 * 1024), Buffer.alloc(1 * 1024) ]
  const hash = new Hash()
  hash.setup('sha1', wb, rb)

  const middleware = (req, res, context) => {
    const { client, parser } = context
    const bytes = new Uint8Array(context.in.bytes)
    req.onEnd(() => {
      if (req.method !== 'GET') {
        res.statusCode = 400
        res.end()
        return
      }
      if (req.upgrade) {
        const chunks = []
        const websocket = new WSParser()
        process.onMessage(message => {
          client.write(createMessage(context.out, message.payload))
        })
        websocket.onHeader = header => (chunks.length = 0)
        websocket.onChunk = (off, len, header) => {
          let size = len
          let pos = off
          while (size--) {
            bytes[pos] = bytes[pos] ^ header.maskkey[(pos - off) % 4]
            pos++
          }
          chunks.push(context.in.read(off, len))
        }
        websocket.onMessage = header => {
          const str = chunks.join('')
          try {
            JSON.parse(str)
            process.sendString(str)
          } catch (err) {
            print('************************* weird one ***************************')
          }
        }
        client.websocket = websocket
        parser.pause()
        const headers = {}
        req.headers.split('\r\n').filter(v => v).map(h => h.split(':')).forEach(pair => (headers[pair[0].trim().toLowerCase()] = pair[1].trim()))
        const key = headers['sec-websocket-key']
        hash.create(wb.write(key))
        hash.update(wb.write('258EAFA5-E914-47DA-95CA-C5AB0DC85B11'))
        res.statusCode = 101
        res.setHeader('Upgrade', 'websocket')
        res.setHeader('Connection', 'Upgrade')
        res.setHeader('Sec-WebSocket-Accept', buf2b64(rb, hash.digest()))
        res.end()
        client.onRead(len => {
          client.onRead(len => websocket.execute(bytes, 0, len))
        })
        return
      }
      let payload
      switch (req.url) {
        case '/json':
        case '/json/list':
          res.statusCode = 200
          res.version = '1.0'
          res.setHeader('Content-Type', 'application/json; charset=UTF-8')
          payload = [{
            description: 'node.js instance',
            devtoolsFrontendUrl: 'chrome-devtools://devtools/bundled/js_app.html?experiments=true&v8only=true&ws=127.0.0.1:9222/81e26cff-e4c5-40a4-927e-b8e2bef06dac',
            devtoolsFrontendUrlCompat: 'chrome-devtools://devtools/bundled/inspector.html?experiments=true&v8only=true&ws=127.0.0.1:9222/81e26cff-e4c5-40a4-927e-b8e2bef06dac',
            faviconUrl: 'https://nodejs.org/static/favicon.ico',
            id: '81e26cff-e4c5-40a4-927e-b8e2bef06dac',
            title: `dv8[${process.PID}]`,
            type: 'node',
            url: 'file://',
            webSocketDebuggerUrl: 'ws://127.0.0.1:9222/81e26cff-e4c5-40a4-927e-b8e2bef06dac'
          }]
          res.end(JSON.stringify(payload))
          break
        case '/json/version':
          res.statusCode = 200
          res.version = '1.0'
          res.setHeader('Content-Type', 'application/json; charset=UTF-8')
          payload = {
            Browser: 'node.js/v10.10.0',
            'Protocol-Version': '1.1'
          }
          res.end(JSON.stringify(payload))
          break
        default:
          res.statusCode = 404
          res.end()
          break
      }
    })
  }

  createServer(middleware, { address: '0.0.0.0', port: 9222 }).listen()
}

const UV_RUN_ONCE = 1
let thread
let looping = false

module.exports = {
  start: () => {
    thread = process.spawn(threadFunc, result => thread.sock.close(), { ipc: true })
    thread.onMessage(message => global.send(message.payload))
    global.receive = message => thread.sendString(message)
    global.onRunMessageLoop = () => {
      if (looping) return
      looping = true
      process.loop.run(1)
      while (looping) {
        process.usleep(1000) // sleep for a millisecond
        process.loop.run(UV_RUN_ONCE) // run the loop once
      }
    }
    global.onQuitMessageLoop = () => (looping = false)
  },
  stop: () => {

  }
}
