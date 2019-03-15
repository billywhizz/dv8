const { Socket, UNIX } = module('socket', {})
const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
const { WSParser, createMessage } = require('../lib/websocket.js')
process.onMessage(message => {
  const payload = JSON.parse(message.payload)
  const { fd } = payload
  if (!fd) return
  const client = new Socket(UNIX)
  const bytes = new Uint8Array(rb.bytes)
  const websocket = new WSParser()
  const chunks = []
  websocket.onHeader = header => (chunks.length = 0)
  websocket.onChunk = (off, len, header) => chunks.push(rb.read(off, len))
  websocket.onMessage = header => {
    const str = chunks.join('')
    print(str)
    setTimeout(() => {
      client.write(createMessage(wb, 'child.ping'))
    }, 1000)
  }
  client.onConnect(() => {
    client.setup(rb, wb)
    client.resume()
    client.write(createMessage(wb, 'child.ping'))
  })
  client.onClose(() => process.sock.close())
  client.onRead(len => websocket.execute(bytes, 0, len))
  client.open(fd)
})
