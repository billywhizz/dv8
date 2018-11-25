function Worker() {
  const rate = { send: 0, recv: 0 }
  const timer = setInterval(() => {
    print(`thread: send ${rate.send} recv ${rate.recv}`)
    rate.send = rate.recv = 0
  }, 1000)
  const payload = JSON.stringify({ TID: process.TID })
  process.onMessage(message => {
    rate.recv++
    process.sendString(payload)
    rate.send++
  })
  process.sendString(payload)
}

const payload = JSON.stringify({ PID: process.PID })
const rate = { send: 0, recv: 0 }
const timer = setInterval(() => {
  print(`process: send ${rate.send} recv ${rate.recv}`)
  rate.send = rate.recv = 0
}, 1000)

function spawn() {
  const thread = process.spawn(Worker, result => console.log(`done: ${result.status}`))
  thread.onMessage(message => {
    rate.recv++
    thread.sendString(payload)
    rate.send++
  })
}

spawn()
