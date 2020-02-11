const { library, print } = dv8
const { UDP } = library('udp')

function createSocket() {
  const sock = new UDP()
  const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
  sock.setup(rb, wb)
  sock.onClose(() => print('close'))
  sock.onMessage((len, address, port) => {
    const message = rb.read(0, len)
    sock.send(wb.write('hello', 0), address, port)
    print(`message from ${address}:${port}, ${len}\n${message}`)
  })
  sock.bind('0.0.0.0', 5555)
  sock.start()
  return sock
}

function createSocket () {
  return sock
}

const server = createSocket()
const client = createSocket()

server.send(wb.write('hello', 0), '', 4444)
