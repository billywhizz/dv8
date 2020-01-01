const ENV = global.env().map(entry => entry.split('=')).reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})
if (ENV.DV8_MODULES) {
  const _library = global.library
  global.library = (name, exports) => _library(name, exports, ENV.DV8_MODULES)
}
const { Process } = library('process', {})
const { Timer } = library('timer', {})
const { Thread } = library('thread', {})
const { EventLoop } = library('loop', {})
const { Socket, UNIX } = library('socket', {})
const { File, O_RDONLY } = library('fs', {})
const { UV_TTY_MODE_NORMAL, TTY } = library('tty', {})

function repl () {
  const BUFFER_SIZE = 64 * 1024
  const MAX_BUFFER = 4 * BUFFER_SIZE
  const stdin = new TTY(0)
  const buf = Buffer.alloc(BUFFER_SIZE)
  function parse (statement) {
    return Function(`"use strict"; return (${statement})`)() // eslint-disable-line no-new-func
  }
  stdin.setup(buf, UV_TTY_MODE_NORMAL)
  stdin.onRead(len => {
    const source = buf.read(0, len)
    try {
      const result = parse(source)
      let payload = `${JSON.stringify(result, null, 2)}\n`
      if (!result) payload = '(null)\n'
      if (!payload) payload = '(null)\n'
      if (payload === undefined) payload = '(undefined)\n'
      const r = stdout.write(buf.write(payload, 0))
      if (r < 0) return stdout.close()
    } catch (err) {
      print(err.message)
    }
    const r = stdout.write(buf.write('> ', 0))
    if (r < 0) return stdout.close()
    if (r < len && stdout.queueSize() >= MAX_BUFFER) stdin.pause()
  })
  stdin.onEnd(() => stdin.close())
  stdin.onClose(() => stdout.close())
  const stdout = new TTY(1)
  stdout.setup(buf, UV_TTY_MODE_NORMAL)
  stdout.onDrain(() => stdin.resume())
  stdout.onError((e, message) => print(`stdout.error:\n${e.toString()}\n${message}`))
  if (stdout.write(buf.write('> ', 0)) < 0) {
    stdout.close()
  } else {
    stdin.resume()
  }
}

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

global.readFile = readFile

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
/*
Error.prepareStackTrace = (err, stack) => {
  const result = []
  let fName
  let lNumber
  for (const callsite of stack) {
    const typeName = callsite.getTypeName()
    const functionName = callsite.getFunctionName()
    const methodName = callsite.getMethodName()
    const scriptName = callsite.getFileName()
    if (scriptName && !fName) fName = scriptName
    const line = callsite.getLineNumber()
    if (line && !lNumber) lNumber = line
    const column = callsite.getColumnNumber()
    const isToplevel = callsite.isToplevel()
    const isEval = callsite.isEval()
    const isWasm = false
    const isNative = callsite.isNative()
    const isUserJavascript = true
    const isConstructor = callsite.isConstructor()
    result.push({ typeName, functionName, methodName, scriptName, line, column, isToplevel, isEval, isNative, isConstructor, isWasm, isUserJavascript })
  }
  Object.defineProperty(err, 'stack', {
    value: result,
    writable: false,
    enumerable: true
  });  
  Object.defineProperty(err, 'lineNumber', {
    value: lNumber,
    writable: false,
    enumerable: true
  });  
  Object.defineProperty(err, 'fileName', {
    value: fName,
    writable: false,
    enumerable: true
  });  
  return result
}

global.onUncaughtException = err => {
  const stack = []
  err.stack.forEach(v => stack.push(v))
  delete err.stack
  err.stack = stack
  print(`onUncaughtException:\n${JSON.stringify(err, null, '  ')}`)
}

global.onUnhandledRejection((promise, err, eventType) => {
  const keys = Object.getOwnPropertyNames(value)
  for (const key of keys) {
    if (key === 'stack') {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${JSON.stringify(value[key], null, '  ')}`)
    } else {
      print(`${key.padEnd(20, ' ').slice(0, 20)} : ${value[key]}`)
    }
  }
})
*/
Error.prepareStackTrace = (err, stack) => {
  const result = []
  for (const callsite of stack) {
    const typeName = callsite.getTypeName()
    const functionName = callsite.getFunctionName()
    const methodName = callsite.getMethodName()
    const scriptName = callsite.getFileName()
    const line = callsite.getLineNumber()
    const column = callsite.getColumnNumber()
    const isToplevel = callsite.isToplevel()
    const isEval = callsite.isEval()
    const isWasm = false
    const isNative = callsite.isNative()
    const isUserJavascript = true
    const isConstructor = callsite.isConstructor()
    result.push({ typeName, functionName, methodName, scriptName, line, column, isToplevel, isEval, isNative, isConstructor, isWasm, isUserJavascript })
  }
  Object.defineProperty(err, 'frames', {
    value: result,
    writable: false,
    enumerable: true
  });  
  Object.defineProperty(err, 'fileName', {
    value: result[0].scriptName,
    writable: false,
    enumerable: true
  });
  Object.defineProperty(err, 'lineNumber', {
    value: result[0].line,
    writable: false,
    enumerable: true
  });
  if (!Object.getOwnPropertyDescriptor(err, 'type')) {
    Object.defineProperty(err, 'type', {
      value: 'GeneralException',
      writable: true,
      enumerable: true
    });
  }
  return err.stack
}

global.onUncaughtException = err => {
  Object.defineProperty(err, 'type', {
    value: 'UncaughtException',
    writable: false,
    enumerable: true
  })
  const stack = err.stack
  if (process.onUncaughtException) return process.onUncaughtException(err) 
  print(`${err.type}\n${stack}`)
}

global.onUnhandledRejection = (err, promise) => {
  Object.defineProperty(err, 'type', {
    value: 'UnhandledRejection',
    writable: false,
    enumerable: true
  })
  Object.defineProperty(err, 'promise', {
    value: promise,
    writable: false,
    enumerable: true
  })
  const stack = err.stack
  if (process.onUncaughtException) return process.onUncaughtException(err) 
  print(`${err.type}\n${stack}`)
}

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
process.cwd = () => _process.cwd()
process.nanosleep = (seconds, nanoseconds) => _process.nanosleep(seconds, nanoseconds)

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
      if (fun) fun()
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

const _require = global.require

global.require = path => {
  const { text } = readFile(path)
  global.module = { exports: {} }
  _require(path, text)
  return global.module.exports
}

global.runScript = path => {
  const { text } = readFile(path)
  _process.runScript(text, path)
}

global.evalScript = text => {
  _process.runScript(text, 'eval')
}

function runLoop () {
  do {
    loop.run()
  } while (loop.isAlive())
  //loop.close()
}
process.runLoop = runLoop

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
  const { workerSource } = global
  delete global.workerSource
  global.evalScript(workerSource.slice(workerSource.indexOf('{') + 1, workerSource.lastIndexOf('}')))
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
    process.nextTick(() => {
      const r = thread.start(fun, err => {
        delete threads[thread.id]
        onComplete({ err, thread })
      }, thread.buffer)
      if (r !== 0) onComplete({ err: new Error(`Bad Status: ${r}`, thread, 0) })
    })
   return thread
  }
  process.env = ENV
  process.PID = _process.pid()
  process.TID = 0
  process.args = global.args
  process.threads = threads
  if (process.args.length < 2) {
    repl()
  } else {
    global.runScript(process.args[1])
  }
}

runLoop()
