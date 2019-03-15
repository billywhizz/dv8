const { TCP, Socket } = module('socket', {})

let stats = { r: 0, w: 0 }

const BUFSIZE = 131072

const clients = []

function createClient () {
  const client = new Socket(TCP)
  const buf = Buffer.alloc(BUFSIZE)
  client.onConnect(() => {
    client.onClose(() => {
  
    })
    client.onEnd(() => client.close())
    client.onError((code, message) => {
      print(code)
      print(message)
    })
    client.onRead(len => {
      stats.r += len
      client.write(len)
      stats.w += len
    })
    client.setup(buf, buf)
    client.write(BUFSIZE)
    stats.w += BUFSIZE
    return client
  })
  client.connect('0.0.0.0', 3000)
  clients.push(client)
}

function toMiB (val) {
  return Math.floor((val / (1024 * 1024)) * 100) / 100
}

setInterval(() => {
  print(`read: ${toMiB(stats.r)}, write: ${toMiB(stats.w)}`)
  stats.w = stats.r = 0
}, 1000)

createClient()
createClient()
//createClient()
//createClient()
