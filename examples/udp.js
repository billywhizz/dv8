function createPing () {
  const { UDP } = module('udp', {})
  const sock = new UDP()
  const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
  sock.setup(rb, wb)
  sock.onClose(() => {
    print('close')
  })
  let r = sock.bind('0.0.0.0', 0)
  print(`bind: ${r}`)
  sock.onSend(() => {
    print('onSend')
  })
  sock.onMessage((len, address, port) => {
    print(`message from ${address}:${port}, ${len}`)
    print(rb.read(0, len))
    //sock.send(wb.write('ping', 0), '0.0.0.0', 30000)
  })
  setInterval(() => {
    sock.send(wb.write('ping', 0), '0.0.0.0', 30000)
  }, 1000)
  sock.start()
}

function createPong () {
  const { UDP } = module('udp', {})
  const sock = new UDP()
  const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
  sock.setup(rb, wb)
  let r = sock.bind('0.0.0.0', 30000)
  print(`bind: ${r}`)
  sock.onSend(() => {
    print('onSend')
  })
  sock.onMessage((len, address, port) => {
    print(`message from ${address}:${port}, ${len}`)
    print(rb.read(0, len))
    sock.send(wb.write('pong', 0), address, port)
  })
  sock.onClose(() => print('close'))
  sock.start()
}

process.spawn(createPong, result => print(`pong thread ${result.thread.id} done`))
process.spawn(createPing, result => print(`ping thread ${result.thread.id} done`))
