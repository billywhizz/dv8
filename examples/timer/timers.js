const { Timer } = module('timer', {})
/*
function createTimer() {
  const t = new Timer()
  t.start(() => {
    print('hello')
    t.close()
  }, 1000)
}

createTimer()
createTimer()
createTimer()
createTimer()
*/

let shutdown = false

function repeater2() {
  t.close()
  if (!shutdown) {
    t = new Timer()
    t.start(repeater2, 1)
  }
}
let t = new Timer()
t.start(repeater2, 0)

setTimeout(() => (shutdown = true), 60000)
/*
function repeater() {
  t.start(repeater, 0)
}
const t = new Timer()
t.start(repeater, 1000)

*/