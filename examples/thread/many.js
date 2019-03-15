function idleThread () {
  const thread = process.spawn(() => {
    const timer = setInterval(() => {

    }, 1000)
  }, result => {
    print('thread says goodbye')
    print(JSON.stringify(process.memoryUsage()))
  }, { ipc: false })
  return thread
}
const threads = []
let i = 50
function next () {
  threads.push(idleThread())
  if (i > 0) {
    i--
    process.nextTick(next)
  }
}

next()
