const { Process } = library('process', {})
const { Timer } = library('timer', {})
const { Thread } = library('thread', {})
const { EventLoop } = library('loop', {})
const { Socket, UNIX } = library('socket', {})
const { File, O_RDONLY } = library('fs', {})

function readFile (path) {
  const buf = Buffer.alloc(16384)
  const file = new File()
  const parts = []
  file.setup(buf, buf)
  file.fd = file.open(path, O_RDONLY)
  file.size = 0
  let len = file.read(16384, file.size)
  while (len > 0) {
    file.size += len
    parts.push(buf.read(0, len))
    len = file.read(16384, file.size)
  }
  file.close()
  file.text = parts.join('')
  return file
}

class Parser {
  constructor (rb, wb) {
    this.position = 0
    this.message = {
      opCode: 0,
      threadId: 0,
      payload: ''
    }
    this.rb = rb
    this.wb = wb
    this.view = {
      recv: new DataView(rb.bytes),
      send: new DataView(wb.bytes)
    }
    this.onMessage = () => { }
  }

  read (len, off = 0) {
    const { rb, view, message } = this
    let { position } = this
    const { recv } = view
    while (off < len) {
      if (position === 0) {
        message.threadId = recv.getUint8(off++)
        position++
      } else if (position === 1) {
        message.opCode = recv.getUint8(off++)
        position++
      } else if (position === 2) {
        message.length = recv.getUint8(off++) << 8
        position++
      } else if (position === 3) {
        message.length += recv.getUint8(off++)
        position++
        message.payload = ''
      } else {
        let toread = message.length - (position - 4)
        if (toread + off > len) {
          toread = len - off
          if (message.opCode !== 3) {
            message.payload += rb.read(off, toread)
          }
          position += toread
          off = len
        } else {
          if (message.opCode !== 3) {
            message.payload += rb.read(off, toread)
            this.onMessage(Object.assign({}, message))
          } else {
            message.payload = null
            message.offset = off
            this.onMessage(Object.assign({}, message))
          }
          off += toread
          position = 0
        }
      }
    }
    this.position = position
  }

  write (o, opCode = 1, off = 0, size = 0) {
    const { wb, view } = this
    const { send } = view
    if (opCode === 1) { // JSON
      const message = JSON.stringify(o)
      const len = message.length
      send.setUint8(off, process.TID || process.PID)
      send.setUint8(off + 1, opCode)
      send.setUint16(off + 2, len)
      wb.write(message, off + 4)
      return len + 4
    } else if (opCode === 2) { // String
      const len = o.length
      send.setUint8(off, process.TID || process.PID)
      send.setUint8(off + 1, opCode)
      send.setUint16(off + 2, len)
      wb.write(o, off + 4)
      return len + 4
    } else if (opCode === 3) { // buffer
      send.setUint8(off, process.TID || process.PID)
      send.setUint8(off + 1, opCode)
      send.setUint16(off + 2, size)
      return size + 4
    }
    return 0
  }
}

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
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4),
  new Float64Array(4)
]

Error.stackTraceLimit = Infinity

Error.prepareStackTrace = (err, stack) => {
  err.stack = []
  for (const callsite of stack) {
    const typeName = callsite.getTypeName()
    const functionName = callsite.getFunctionName()
    const methodName = callsite.getMethodName()
    const fileName = callsite.getFileName()
    const lineNumber = callsite.getLineNumber()
    const columnNumber = callsite.getColumnNumber()
    const isToplevel = callsite.isToplevel()
    const isEval = callsite.isEval()
    const isNative = callsite.isEval()
    const isConstructor = callsite.isConstructor()
    err.stack.push({ typeName, functionName, methodName, fileName, lineNumber, columnNumber, isToplevel, isEval, isNative, isConstructor })
  }
}

global.onUncaughtException = err => {
  print('onUncaughtException:')
  const keys = Object.getOwnPropertyNames(err)
  for (const key of keys) {
    if (key === 'stack') {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${JSON.stringify(err[key], null, '  ')}`)
    } else {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${err[key]}`)
    }
  }
}

global.onUnhandledRejection((promise, value, eventType) => {
  print('onUnhandledRejection:')
  const keys = Object.getOwnPropertyNames(value)
  for (const key of keys) {
    if (key === 'stack') {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${JSON.stringify(value[key], null, '  ')}`)
    } else {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${value[key]}`)
    }
  }
})

function setTimeout (fn, delay) {
  const t = new Timer()
  t.start(() => {
    fn()
    t.close()
  }, delay)
  return t
}

function setInterval (fn, repeat) {
  const t = new Timer()
  t.start(fn, repeat, repeat)
  return t
}

function clearTimeout (t) {
  t.stop()
  t.close()
}

const GlobalBuffer = global.Buffer
global.Buffer = {
  alloc: size => {
    const buf = new GlobalBuffer()
    const ab = buf.alloc(size)
    buf.size = size
    buf.bytes = ab
    return buf
  },
  fromString: str => {
    const buf = global.Buffer.alloc(str.length)
    buf.write(str)
    return buf
  }
}

const _process = new Process()
const process = {}
const loop = new EventLoop()
process.loop = loop

process.sleep = seconds => _process.sleep(seconds)
process.usleep = microseconds => _process.usleep(microseconds)

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
  return _process.heapUsage(heap)
}

process.memoryUsage = () => {
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

global.setTimeout = setTimeout
global.setInterval = setInterval
global.clearTimeout = clearTimeout
global.clearInterval = clearTimeout
global.console = { log: global.print, error: global.print, dir: o => print(JSON.stringify(o, null, '  ')) }
global.process = process

process.runMicroTasks = () => _process.runMicroTasks()
process.ticks = 0

let idleActive = false

process.stats = () => {
  const st = {
    ticks: process.ticks,
    queue: queue.length
  }
  return st
}

const activeHandles = () => {
  const buf = Buffer.alloc(16384)
  loop.listHandles(buf)
  const bytes = new Uint8Array(buf.bytes)
  let off = 0
  const handles = []
  while (1) {
    const active = bytes[off]
    const len = bytes[off + 1]
    if (len === 0) break
    const type = buf.read(off + 2, len)
    handles.push({ active, type })
    off += (2 + len)
  }
  return handles
}

process.activeHandles = activeHandles

const nextTick = fn => {
  queue.push(fn)
  if (idleActive) return
  loop.onIdle(() => {
    process.ticks++
    let len = queue.length
    while (len--) {
      const fun = queue.shift()
      try {
        fun()
      } catch (err) {
        throw (err)
      }
    }
    if (!queue.length) {
      idleActive = false
      loop.onIdle()
    }
  })
  loop.ref() // ensure we run
  idleActive = true
}
// TODO: rename this as it's not the same as node.js nextTick
process.nextTick = nextTick

global.onExit(() => {
  // at this point the main loop is done so if you want to run anything you have to manually pump the loop
  process.loop.reset()
  process.loop.run(2)
  if (process.onExit) process.onExit()
})

global.require = path => {
  const module = { exports: {} }
  const { text } = readFile(path)
  const source = `(function (module) {\n${text}\n})(module)`
  eval(source)
  return module.exports
}

global.runScript = path => {
  const { text } = readFile(path)
  const source = `(function (global) {\n${text}\n})(global)`
  eval(source)
}

global.evalScript = text => {
  const source = `(function (global) {\n${text}\n})(global)`
  eval(source)
}

if (global.workerData) {
  global.workerData.bytes = global.workerData.alloc()
  const dv = new DataView(global.workerData.bytes)
  process.TID = dv.getUint8(0)
  process.PID = _process.pid()
  process.fd = dv.getUint32(1)
  const envLength = dv.getUint32(5)
  const envJSON = global.workerData.read(9, envLength)
  process.env = JSON.parse(envJSON)
  const argsLength = dv.getUint32(9 + envLength)
  const argsJSON = global.workerData.read(13 + envLength, argsLength)
  process.args = JSON.parse(argsJSON)
  delete global.workerData
  if (process.fd !== 0) {
    const bufSize = parseInt(process.env.THREAD_BUFFER_SIZE || 1024, 10)
    const [rb, wb] = [Buffer.alloc(bufSize), Buffer.alloc(bufSize)]
    const sock = new Socket(UNIX)
    const parser = new Parser(rb, wb)
    sock.onConnect(() => {
      sock.setup(rb, wb)
      sock.resume()
    })
    parser.onMessage = message => sock.onMessage(message)
    // TODO: messages too big
    process.send = o => sock.write(parser.write(o))
    process.sendString = s => {
      sock.write(parser.write(s, 2))
    }
    process.sendBuffer = len => sock.write(parser.write(null, 3, 0, len))
    sock.onRead(len => parser.read(len))
    sock.onEnd(() => sock.close())
    process.onMessage = fn => {
      sock.onMessage = fn
    }
    sock.open(process.fd)
    process.sock = sock
  }
  let alive = true
  const { workerSource } = global
  delete global.workerSource
  global.evalScript(workerSource)
  do {
    loop.run()
    alive = loop.isAlive()
  } while (alive)
  loop.close()
} else {
  process.spawn = (fun, onComplete, opts = { ipc: false }) => {
    const thread = new Thread()
    const envJSON = JSON.stringify(process.env)
    const argsJSON = JSON.stringify(process.args)
    const bufferSize = envJSON.length + argsJSON.length + 13
    thread.buffer = Buffer.alloc(bufferSize)
    const view = new DataView(thread.buffer.bytes)
    if (opts.ipc) {
      const bufSize = parseInt(process.env.THREAD_BUFFER_SIZE || 1024, 10)
      const [rb, wb] = [Buffer.alloc(bufSize), Buffer.alloc(bufSize)]
      const sock = new Socket(UNIX)
      const parser = new Parser(rb, wb)
      sock.onConnect(() => {
        sock.setup(rb, wb)
        sock.resume()
      })
      sock.onEnd(() => sock.close())
      sock.onRead(len => parser.read(len))
      parser.onMessage = message => thread._onMessage(message)
      thread.onMessage = fn => {
        thread._onMessage = fn
      }
      thread._onMessage = message => {}
      thread.send = o => sock.write(parser.write(o))
      thread.sendString = s => {
        sock.write(parser.write(s, 2))
      }
      thread.sendBuffer = len => sock.write(parser.write(null, 3, 0, len))
      const fd = sock.open()
      if (fd < 0) {
        throw new Error(`Error: ${fd}: ${sock.error(fd)}`)
      }
      view.setUint32(1, fd)
      thread.sock = sock
    } else {
      view.setUint32(1, 0)
    }
    thread.view = view
    thread.id = next++
    view.setUint8(0, thread.id)
    view.setUint32(5, envJSON.length)
    thread.buffer.write(envJSON, 9)
    view.setUint32(envJSON.length + 9, argsJSON.length)
    thread.buffer.write(argsJSON, envJSON.length + 13)
    threads[thread.id] = thread
    // TODO: do we need nextTick here? for the ipc socket ?
    nextTick(() => {
      thread.start(fun, (err, status) => onComplete({ err, thread, status }), thread.buffer)
    })
    return thread
  }
  process.env = global.env().map(entry => entry.split('=')).reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})
  process.PID = _process.pid()
  process.TID = 0
  process.args = global.args
  process.threads = threads
  if (process.args.length < 2) {
    // repl?
  } else {
    let alive = true
    global.runScript(process.args[1])
    do {
      loop.run()
      alive = loop.isAlive()
    } while (alive)
    loop.close()
  }
}
