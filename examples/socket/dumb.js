const dbg = require('../debugger/debug.js')
dbg.start()

const { hexy } = require('../lib/hexy.js')
const { TCP, Socket } = module('socket', {})
const { OS } = module('os', {})

const SIGTERM = 15

const os = new OS()

process.version = 'v11.4.0'
process.arch = 'x64'
process.pid = process.PID

function terminateHandler (signum) {
  console.log('terminating')
  dbg.stop()
  // stop the listening socket
  sock.close()
  const keys = Object.keys(clients)
  for (const k of keys) {
    const client = clients[k]
    delete clients[k]
    client.close(0)
  }
  //shutdown()
  return 1
}

os.onSignal(terminateHandler, SIGTERM)

const buf = Buffer.alloc(16384)
const bytes = new Uint8Array(buf.bytes)

const sock = new Socket(TCP)
let id = 0
const clients = {}

sock.onConnect(() => {
  const client = new Socket(TCP)
  client.id = id++
  clients[client.id] = client
  client.onClose(() => {
    delete clients[client.id]
  })
  client.onEnd(() => client.close())
  client.onError((code, message) => {
    print(code)
    print(message)
  })
  client.onRead(len => {
    print(buf.read(0, len))
    print(hexy(bytes.slice(0, len)))
  })
  client.setup(buf, buf)
  return client
})

sock.listen('0.0.0.0', 3000)
