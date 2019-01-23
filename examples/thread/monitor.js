function spawn () {
  const thread = process.spawn(() => {
    const { OS } = module('os', {})
    const SIGCONT = 18
    const os = new OS()
    os.onSignal(signum => {
      print(`thread(${process.TID}).SIGNAL: ${signum}`)
      running = false
      return 1
    }, SIGCONT)
    process.onExit = () => {
      print('thread.exit')
    }
    let running = true
    function next () {
      //if (running) process.nextTick(next)
      if (running) setTimeout(next, 1000)
    }
    process.nextTick(next)
    process.send({ id: process.TID })
  }, result => {
    print('thread says goodbye')
  }, { ipc: true })
  thread.onMessage(m => {
    const payload = JSON.parse(m.payload)
    payload.boot = Date.now() - thread.started
    print(JSON.stringify(payload))
  })
  thread.started = Date.now()
  return thread
}

let count = parseInt(process.args[2] || 1, 10)
function next () {
  spawn()
  if (count--) process.nextTick(next)
}
next()

//setTimeout(() => thread.stop(), 5000)
/*
thread.lastTime = thread.time()
setInterval(() => {
  const threadTime = thread.time()
  print(threadTime - thread.lastTime)
  thread.lastTime = threadTime
}, 1000)
process.onExit = () => {
  print('main.exit')
}
*/
