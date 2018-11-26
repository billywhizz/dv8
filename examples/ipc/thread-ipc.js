const payload = { PID: process.PID, TID: process.TID }

const thread = process.spawn(() => {
  const payload = { PID: process.PID, TID: process.TID }
  process.onMessage(message => {
    print(JSON.stringify(message))
    process.sock.unref()
  })
  process.send(payload)
}, result => thread.sock.close(), { ipc: true })

thread.onMessage(message => {
  print(JSON.stringify(message))
  thread.send(payload)
  thread.sock.unref()
})
