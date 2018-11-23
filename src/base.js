/*
A PoC base module which exposes timers and memoryUsage
*/
const { Process } = module('process', {})
const { Timer } = module('timer', {})
const { Thread } = module('thread', {})
const { EventLoop } = module('loop', {})

/*
Locals
*/
const mem = new Float64Array(16)
const cpu = new Float64Array(2)
const time = new BigInt64Array(1)
let next = 1
const queue = []
const threads = {}
const heap = [
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4)
]

global.onUncaughtException = err => {
  print('onUncaughtException:')
  print(JSON.stringify(err))
}

global.onUnhandledRejection(err => {
  print('onUnhandledRejection:')
  print(JSON.stringify(err))
})

function setTimeout (fn, delay) {
  const t = new Timer()
  // the module will make the callback after delay
  t.start(fn, delay)
  // return the timer so it can be stopped. no gc/unref handling yet!
  return t
}

function setInterval (fn, repeat) {
  const t = new Timer()
  t.start(fn, repeat, repeat)
  return t
}

function clearTimeout (t) {
  // this will close the libuv handle for the timer
  t.stop()
}

/*
Global Buffer instance
*/
const GlobalBuffer = global.Buffer
global.Buffer = {
  alloc: size => {
    const buf = new GlobalBuffer()
    const ab = buf.alloc(size)
    buf.bytes = ab
    return buf
  }
}

/*
Process
*/
const _process = new Process()
const process = {}

/*
Event Loop
*/
const loop = new EventLoop()
process.loop = loop
let THREAD_BUFFER_SIZE = 1024

process.cpuUsage = () => {
  _process.cpuUsage(cpu)
  return {
    user: cpu[0],
    system: cpu[1]
  }
}

process.hrtime = () => {
  _process.hrtime(time)
  return time[0]
}

process.heapUsage = () => {
  // read the values into the float array
  return _process.heapUsage(heap)
}

process.memoryUsage = () => {
  // read the values into the float array
  _process.memoryUsage(mem)
  return {
    rss: mem[0],
    total_heap_size: mem[1],
    used_heap_size: mem[2],
    external_memory: mem[3],
    does_zap_garbage: mem[4],
    heap_size_limit: mem[5],
    malloced_memory: mem[6],
    number_of_detached_contexts: mem[7],
    number_of_native_contexts: mem[8],
    peak_malloced_memory: mem[9],
    total_available_size: mem[10],
    total_heap_size_executable: mem[11],
    total_physical_size: mem[12],
    isolate_external: mem[13]
  }
}

process.spawn = (fun, onComplete) => {
  const thread = new Thread()
  thread.buffer = Buffer.alloc(THREAD_BUFFER_SIZE)
  const view = new DataView(thread.buffer.bytes)
  thread.view = view
  thread.id = next++
  view.setUint8(0, thread.id)
  const envJSON = JSON.stringify(process.env)
  const argsJSON = JSON.stringify(process.args)
  view.setUint32(5, envJSON.length)
  thread.buffer.write(envJSON, 9)
  view.setUint32(envJSON.length + 9, argsJSON.length)
  thread.buffer.write(argsJSON, envJSON.length + 13)
  thread.start(fun, (err, status) => onComplete({ err, thread, status }), thread.buffer)
  threads[thread.id] = thread
  return thread
}

// JS Globals
global.setTimeout = setTimeout
global.setInterval = setInterval
global.clearTimeout = clearTimeout
global.clearInterval = clearTimeout
global.console = { log: global.print }
global.process = process

process.runMicroTasks = () => _process.runMicroTasks()
process.ticks = 0
process.nextTick = fn => {
  queue.push(fn)
  if (queue.length > 1) return
  loop.onIdle(() => {
    process.ticks++
    let len = queue.length
    while (len--) queue.shift()()
    if (!queue.length) loop.onIdle()
  })
}

/*
Initialize Isolate Thread

global.workerData is set from C++ by the thread module and set in
the global object on the new isolate. it will not exist unless in a
thread isolate
global.workerData is an instance of builtin Buffer
*/
if (global.workerData) {
  // a thread should be sandboxed even further as it can run untrusted code
  // we need to remove access to anything that could crash the process or do damage
  // to the system
  // this is a hack to get the buffer allocated from main isolate
  // would be nice to have IPC and heartbeats out of the box with threads
  // also need to be able to run cpu intensive code in threads which will block the thread event loop
  // can we ue microTasks somehow to allow v8 to do IPC while isolate thread is busy?
  global.workerData.bytes = global.workerData.alloc()
  const dv = new DataView(global.workerData.bytes)
  process.TID = dv.getUint8(0)
  process.PID = _process.pid()
  process.fd = dv.getUint32(1)
  // read the environment and args from the thread buffer
  const envLength = dv.getUint32(5)
  const envJSON = global.workerData.read(9, envLength)
  process.env = JSON.parse(envJSON)
  const argsLength = dv.getUint32(9 + envLength)
  const argsJSON = global.workerData.read(13 + envLength, argsLength)
  process.args = JSON.parse(argsJSON)
  // set the boot time
  delete global.workerData
} else {
  process.env = global.env().map(entry => entry.split('=')).reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})
  process.PID = _process.pid()
  process.args = global.args
  process.threads = threads
}
if (process.env.THREAD_BUFFER_SIZE) {
  THREAD_BUFFER_SIZE = parseInt(process.env.THREAD_BUFFER_SIZE, 10)
}

module.exports = {}
