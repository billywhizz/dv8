const { Socket, UNIX } = module('socket', {})
const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
const { WSParser, createMessage } = require('../lib/websocket.js')
const thread = process.spawn('./child.js', result => thread.sock.close(), { ipc: true })
thread.onMessage(message => print(JSON.stringify(message)))
const server = new Socket(UNIX)
const bytes = new Uint8Array(rb.bytes)
const websocket = new WSParser()
const chunks = []
websocket.onHeader = header => (chunks.length = 0)
websocket.onChunk = (off, len, header) => chunks.push(rb.read(off, len))
websocket.onMessage = header => {
  const str = chunks.join('')
  print(str)
  setTimeout(() => {
    server.write(createMessage(wb, 'parent.pong'))
  }, 1000)
}
server.onConnect(() => {
  server.setup(rb, wb)
  server.resume()
})
server.onRead(len => websocket.execute(bytes, 0, len))
thread.send({ fd: server.open() })
