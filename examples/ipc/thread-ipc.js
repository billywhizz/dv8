const payload = JSON.stringify({ PID: process.PID })

const thread = process.spawn(() => {
  const payload = JSON.stringify({ TID: process.TID })
  const len = process.ipc.out.write(payload, 4)
  process.onMessage(message => {
    print(JSON.stringify(message))
    print(process.ipc.in.read(message.offset, message.length))
    process.sock.unref()
  })
  process.sendBuffer(len)
}, result => thread.sock.close())

const len = process.ipc.out.write(payload, 4)
thread.onMessage(message => {
  print(JSON.stringify(message))
  print(process.ipc.in.read(message.offset, message.length))
  thread.sendBuffer(len)
  thread.sock.unref()
})
