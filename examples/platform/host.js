require('./lib/ipc.js')

function Worker() {
  require('./lib/ipc.js')
  const { env, args, PID, TID } = process
  process.onMessage(message => {
    console.log(`process: ${JSON.stringify(message)}`)
  })
  setTimeout(() => {
    process.send({ env, args, PID, TID })
  }, 1000)
}

const { env, args, PID, TID } = process
const thread = process.spawn(Worker, result => console.log(`done: ${result.status}`))
thread.onMessage(message => {
  console.log(`thread: ${JSON.stringify(message)}`)
  thread.send({ env, args, PID, TID })
})
