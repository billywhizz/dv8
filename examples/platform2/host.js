require('./lib/ipc.js')

process.onMessage(message => {
  console.log(JSON.stringify(message))
  setTimeout(() => {
    const { env, args } = process
    process.send({ env, args, PID, TID })
  }, 1000)
})

spawn('./worker.js', { respawn: true })
spawn('./worker.js')
spawn('./worker.js')
spawn('./worker.js')

const timer = setInterval(() => {
  const running = Object.keys(threads).length
  console.log(`running: ${running}`)
  if (running === 0) {
    clearTimeout(timer)
    listener.close()
  }
}, 1000)


const { Socket, UNIX, TCP } = module('socket', {})
const { IPC } = require('./lib/ipc.js')

/*
each host gets a port range
each isolate can have any number of ports within a range
port counter
queue of ports
increment port counter for each new port
when each isolate ends, psuh the port onto queue
when port counter reaches limit, shift the top element off the array
if array is empty, return error
*/
const ports = []

function createIsolate (opts, onMessage, onEnd) {
  const thread = spawn(opts.path, err => {
    if (err) return onEnd(err)
    if (opts.respawn) {
      process.nextTick(() => createIsolate(opts, onMessage, onEnd))
    }
    onEnd()
  })
}

const demo = {
  path: './worker.js',
  env: {
    THREAD_BUFFER_SIZE: 1024
  },
  args: [],
  limits: {
    cpu: {},
    mem: {},
    net: {}
  },
  listeners: [{
    port: 3000,
    type: TCP
  }],
  respawn: true,
  modules: ['os', 'socket']
}

createIsolate(demo)
createIsolate(demo)
createIsolate(demo)

IPC.onMessage(message => {

})

IPC.send({ env: process.env, PID: process.PID, TID: process.TID })
const { Socket, UNIX } = module('socket', {})

const threads = {}

function spawn(fn, opts = { respawn: false }) {
  const { respawn } = opts
  const thread = createThread(fn, ({ err, thread, status }) => {
    delete threads[thread.id]
    console.log(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}\nstatus: ${status}`)
    if (err) {
      console.log(err.message)
      console.log(err.stack)
      return
    }
    if (respawn) process.nextTick(() => spawn(fn, opts))
  })
  threads[thread.id] = thread
  return thread
}

const listener = new Socket(UNIX)

listener.onConnect(fd => {
  const peer = new Socket(UNIX)
  const [rb, wb] = [createBuffer(16384), createBuffer(16384)]
  const parser = new Parser(rb, wb)
  peer.setup(fd, rb, wb)
  peer.onRead(len => {
    parser.read(len)
  })
  peer.onEnd(() => peer.close())
  parser.onMessage = message => {
    console.log(`main recv from ${message.threadId}`)
    const o = JSON.parse(message.payload)
    console.log(JSON.stringify(o))
    peer.write(parser.write({ shutdown: true }))
  }
})

