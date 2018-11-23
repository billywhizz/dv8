function threadFunc() {
  process.onMessage(message => {
    if (send < 10) {
      process.send({})
      send++
    } else {
      process.sock.close()
    }
  })
  process.send({})
  let send = 1
}

function onMessage(thread) {
  return message => {
    print(JSON.stringify(message))
    thread.send({})
  }
}

function spawn() {
  const thread = process.spawn(threadFunc, result => thread.sock.close())
  thread.onMessage(onMessage(thread))
}

spawn()
