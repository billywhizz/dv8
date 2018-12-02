const { UDP } = module('udp', {})
const sock = new UDP()
const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
sock.setup(rb, wb)
sock.bind('0.0.0.0', 30000)
sock.onMessage((len, address, port) => {
  print(`message from ${address}:${port}, ${len}`)
  print(rb.read(0, len))
  sock.stop()
  setTimeout(() => {
    sock.start()
  }, 3000)
})
sock.onClose(() => {
  print('close')
})
sock.start()
