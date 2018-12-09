const { OS } = module('os', {})
const { Socket, TCP } = module('socket', {})

const SIGTERM = 15

const os = new OS()

process.version = 'v11.4.0'
process.arch = 'x64'
process.pid = process.PID

function terminateHandler (signum) {
  try {
    console.log('terminating')
    sock.close()
    for (const k of Object.keys(clients)) {
      clients[k].close()
    }
  } catch (err) {
    print(err.message)
    print(err.stack)
  }
  return 1
}

os.onSignal(terminateHandler, SIGTERM)

const clients = {}
let id = 0
const sock = new Socket(TCP)

sock.onConnect(() => {
  const client = new Socket(TCP)
  client.setup(Buffer.alloc(1024), Buffer.alloc(1024))
  client.onClose(() => delete clients[client.id])
  print('connection')
  client.id = id++
  clients[client.id] = client
  return client
})

sock.listen('127.0.0.1', 3000)
