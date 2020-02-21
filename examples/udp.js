const { library, print } = dv8
const { UDP } = library('udp')

function createSocket (port) {
  const sock = new UDP()
  const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
  sock.in = rb
  sock.out = wb
  sock.setup(rb, wb)
  sock.bind('0.0.0.0', port)
  sock.start()
  return sock
}

const server = createSocket(4444)
const client = createSocket(5555)

let id = 0

server.onMessage((len, address, port) => {
  print(`${server.in.read(0, len)}`)
})

server.timer = setInterval(() => {
  client.send(client.out.write(`message: ${id++}`), '127.0.0.1', 4444)
  if (id % 5 === 0) {
    if (server.paused) {
      server.start()
      server.paused = false
    } else {
      server.stop()
      server.paused = true
    }
  }
}, 1000)
