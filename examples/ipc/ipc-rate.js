function Worker() {
  const rate = { send: 0, recv: 0 }
  const len = process.ipc.out.write('ping', 4)
  const timer = setInterval(() => {
    print(`thread: send ${rate.send} recv ${rate.recv}`)
    rate.send = rate.recv = 0
  }, 1000)
  const payload = JSON.stringify({ TID: process.TID })
  process.onMessage(message => {
    rate.recv++
    process.sendBuffer(len)
    rate.send++
  })
  process.sendBuffer(len)
}

const payload = JSON.stringify({ PID: process.PID })
const len = process.ipc.out.write('pong', 4)
const rate = { send: 0, recv: 0 }
const timer = setInterval(() => {
  print(`process: send ${rate.send} recv ${rate.recv}`)
  rate.send = rate.recv = 0
}, 1000)

function spawn() {
  const thread = process.spawn(Worker, result => console.log(`done: ${result.status}`), { ipc: true })
  thread.onMessage(message => {
    rate.recv++
    thread.sendBuffer(len)
    rate.send++
  })
}

spawn()
