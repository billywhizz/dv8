let workers = parseInt(process.args[2] || '1')

const threads = {}

function spawn () {
  const thread = process.spawn(() => {
    const { Socket, TCP } = module('socket', {})
    const server = new Socket(TCP)
    const clients = {}
    const [ rb, wb ] = [ Buffer.alloc(1024), Buffer.alloc(1024) ]
    server.onConnect(fd => {
      const client = new Socket(TCP)
      client.setup(fd, rb, wb)
      clients[fd] = client
      print('client.connect')
      client.onClose(() => {
        print('client.close')
        delete clients[fd]
      })
    })
    process.onMessage(m => {
      print('process.onMessage')
      print(JSON.stringify(m, null, '  '))
      const message = JSON.parse(m.payload)
      if (message.command === 'start') {
        const { port } = message
        const r = server.listen('0.0.0.0', port)
        print(`listen: ${r}`)
        process.send({ listen: r })
      } else if (message.command === 'stop') {
        server.close()
        const keys = Object.keys(clients)
        for (const k of keys) {
          clients[k].close()
        }
        process.nextTick(() => {
          process.sock.close()
        })
      }
    })
  }, result => {
    print(`thread complete: ${thread.id}`)
    delete threads[thread.id]
    clearTimeout(timer)
    spawn()
  }, { ipc: true })
  thread.onMessage(message => {
    print(`thread.onMessage: ${thread.id}`)
    print(JSON.stringify(message, null, '  '))
  })
  threads[thread.id] = thread
  process.nextTick(() => {
    thread.send({ command: 'start', port: 3000 })
  })
  const timer = setTimeout(() => {
    thread.send({ command: 'stop' })
  }, 20000)
}

while (workers--) spawn()
