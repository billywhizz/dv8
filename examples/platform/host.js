function Worker() {
  let rate = 0
  const tt = setInterval(() => {
    print(`thread: ${rate}`)
    rate = 0
  }, 1000)
  const { env, args, PID, TID } = process
  const payload = { env, args, PID, TID }
  process.onMessage(message => {
    process.send(payload)
    rate++
  })
  setTimeout(() => {
    process.send({ env, args, PID, TID })
  }, 1000)
}

const { env, args, PID, TID } = process
const payload = { env, args, PID, TID }
const thread = process.spawn(Worker, result => console.log(`done: ${result.status}`))
let rate = 0
const tt = setInterval(() => {
  print(`process: ${rate}`)
  rate = 0
}, 1000)
thread.onMessage(message => {
  thread.send(payload)
  rate++
})
