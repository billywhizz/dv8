const path = process.args[2]
if (!path) throw new Error('path required')
let workers = parseInt(process.args[3] || '1')
const threads = {}
function spawn () {
  const thread = process.spawn(path, result => {
    delete threads[thread.id]
  }, { ipc: true })
  thread.onMessage(message => {
    print(JSON.stringify(JSON.parse(message.payload), null, '  '))
  })
  threads[thread.id] = thread
}
while (workers--) spawn()
