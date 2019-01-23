function busyThread () {
  const thread = process.spawn(() => {
    const { OS } = module('os', {})
    const SIGCONT = 18
    const os = new OS()
    os.onSignal(signum => {
      print(`thread(${process.TID}).SIGNAL: ${signum}`)
      clearInterval(timer)
      running = false
      return 1
    }, SIGCONT)
    process.onExit = () => {
      print('thread.exit')
    }
    const timer = setInterval(() => {

    }, 1000)
    let running = true
    function foo () {
      if (running) process.nextTick(foo)
    }
    process.nextTick(foo)
  }, result => {
    print('thread says goodbye')
    print(JSON.stringify(process.memoryUsage()))
  }, { ipc: false })
  return thread
}

function idleThread () {
  const thread = process.spawn(() => {
    const { OS } = module('os', {})
    const SIGCONT = 18
    const os = new OS()
    os.onSignal(signum => {
      print(`thread(${process.TID}).SIGNAL: ${signum}`)
      clearInterval(timer)
      return 1
    }, SIGCONT)
    process.onExit = () => {
      print('thread.exit')
    }
    const timer = setInterval(() => {

    }, 1000)
  }, result => {
    print('thread says goodbye')
    print(JSON.stringify(process.memoryUsage()))
  }, { ipc: false })
  return thread
}

const t1 = idleThread()
print(`thread: ${t1.id} created`)
setTimeout(() => t1.stop(), 5000)

const t2 = busyThread()
print(`thread: ${t2.id} created`)
setTimeout(() => t2.stop(), 10000)

let lt1 = 0
let lt2 = 0

setInterval(() => {
  const tt1 = t1.time()
  const tt2 = t2.time()
  print(tt1 - lt1)
  print(tt2 - lt2)
  lt1 = tt1
  lt2 = tt2
}, 1000)

process.onExit = () => {
  print('main.exit')
}
