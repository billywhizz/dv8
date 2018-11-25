const payload = JSON.stringify({ PID: process.PID })

const thread = process.spawn(() => {
  const payload = JSON.stringify({ TID: process.TID })
  process.onMessage(message => {
    print(JSON.stringify(message))
    process.sock.unref()
  })
  process.sendString(payload)
}, result => thread.sock.close())

thread.onMessage(message => {
  print(JSON.stringify(message))
  thread.sendString(payload)
  thread.sock.unref()
})
