const { TCP, Socket } = module('socket', {})

const sock = new Socket(TCP)
const buf = Buffer.alloc(131072)

let stats = { r: 0, w: 0 }

sock.onConnect(() => {
  const client = new Socket(TCP)
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
  return client
})

function toMiB (val) {
  return Math.floor((val / (1024 * 1024)) * 100) / 100
}

setInterval(() => {
  print(`read: ${toMiB(stats.r)}, write: ${toMiB(stats.w)}`)
  stats.w = stats.r = 0
}, 1000)

sock.listen('0.0.0.0', 3000)
