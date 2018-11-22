require('./lib/ipc.js')
process.onMessage(message => {
  console.log(JSON.stringify(message))
  const tt = setTimeout(() => {
    const { env, args } = process
    process.send({ env, args, PID, TID })
  }, 1000)
})
