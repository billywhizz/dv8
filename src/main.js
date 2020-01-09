const ENV = global.env()
  .map(entry => entry.split('='))
  .reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})
if (ENV.DV8_MODULES) {
  const _library = global.library
  global.library = (n, e) => _library(n, e, ENV.DV8_MODULES)
}
const { Process } = library('process', {})
const { Timer } = library('timer', {})
const { Thread } = library('thread', {})
const { EventLoop } = library('loop', {})
const { Socket, UNIX } = library('socket', {})
const { File, O_RDONLY } = library('fs', {})
const { UV_TTY_MODE_NORMAL, TTY } = library('tty', {})

let next = 1
const queue = []
const threads = {}
let idleActive = false
const process = {}
const cache = {}
const mem = new Float64Array(16)
const cpu = new Float64Array(2)
const time = new BigUint64Array(1)
const heap = Array.from(new Array(16)).map(v => new Float64Array(4))
const GlobalBuffer = global.Buffer
Error.stackTraceLimit = 1000 // Infinity
const _process = new Process()
const loop = new EventLoop()

process.loop = loop
process.cwd = (...args) => _process.cwd.apply(_process, args)
process.sleep = (...args) => _process.sleep.apply(_process, args)
process.usleep = (...args) => _process.usleep.apply(_process, args)
process.nanosleep = (...args) => _process.nanosleep.apply(_process, args)
process.runMicroTasks = (...args) => {
  return _process.runMicroTasks.apply(_process, args)
}
global.process = process
process.ticks = 0
global.__dirname = process.cwd()
global.__filename = 'main.js'

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
          if (message.opCode === 1) {
            message.payload += rb.read(off, toread)
            this.onMessage(Object.assign({}, JSON.parse(message.payload)))
          } else if (message.opCode === 2) {
            message.payload += rb.read(off, toread)
            this.onMessage(Object.assign({}, message))
          } else if (message.opCode === 3) {
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
      const message = JSON.stringify(o, replacer)
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

function pathModule () {
  const CHAR_FORWARD_SLASH = 47
  const CHAR_BACKWARD_SLASH = 92
  const CHAR_DOT = 46

  function baseName (path) {
    return path.slice(0, path.lastIndexOf('/') + 1)
  }

  function isPathSeparator (code) {
    return code === CHAR_FORWARD_SLASH || code === CHAR_BACKWARD_SLASH
  }

  function isPosixPathSeparator (code) {
    return code === CHAR_FORWARD_SLASH
  }

  function normalizeString (path, allowAboveRoot, separator) {
    let res = ''
    let lastSegmentLength = 0
    let lastSlash = -1
    let dots = 0
    let code = 0
    for (let i = 0; i <= path.length; ++i) {
      if (i < path.length) {
        code = path.charCodeAt(i)
      } else if (isPathSeparator(code)) {
        break
      } else {
        code = CHAR_FORWARD_SLASH
      }

      if (isPathSeparator(code)) {
        if (lastSlash === i - 1 || dots === 1) {
          // NOOP
        } else if (dots === 2) {
          if (res.length < 2 || lastSegmentLength !== 2 ||
              res.charCodeAt(res.length - 1) !== CHAR_DOT ||
              res.charCodeAt(res.length - 2) !== CHAR_DOT) {
            if (res.length > 2) {
              const lastSlashIndex = res.lastIndexOf(separator)
              if (lastSlashIndex === -1) {
                res = ''
                lastSegmentLength = 0
              } else {
                res = res.slice(0, lastSlashIndex)
                lastSegmentLength = res.length - 1 - res.lastIndexOf(separator)
              }
              lastSlash = i
              dots = 0
              continue
            } else if (res.length !== 0) {
              res = ''
              lastSegmentLength = 0
              lastSlash = i
              dots = 0
              continue
            }
          }
          if (allowAboveRoot) {
            res += res.length > 0 ? `${separator}..` : '..'
            lastSegmentLength = 2
          }
        } else {
          if (res.length > 0) {
            res += `${separator}${path.slice(lastSlash + 1, i)}`
          } else {
            res = path.slice(lastSlash + 1, i)
          }
          lastSegmentLength = i - lastSlash - 1
        }
        lastSlash = i
        dots = 0
      } else if (code === CHAR_DOT && dots !== -1) {
        ++dots
      } else {
        dots = -1
      }
    }
    return res
  }

  function normalize (path) {
    if (path.length === 0) return '.'
    const isAbsolute = path.charCodeAt(0) === CHAR_FORWARD_SLASH
    const sep = path.charCodeAt(path.length - 1) === CHAR_FORWARD_SLASH
    path = normalizeString(path, !isAbsolute, '/', isPosixPathSeparator)
    if (path.length === 0) {
      if (isAbsolute) return '/'
      return sep ? './' : '.'
    }
    if (sep) path += '/'
    return isAbsolute ? `/${path}` : path
  }

  function join (...args) {
    if (args.length === 0) return '.'
    if (args.length === 2 && args[1][0] === '/') return normalize(args[1])
    let joined
    for (let i = 0; i < args.length; ++i) {
      const arg = args[i]
      if (arg.length > 0) {
        if (joined === undefined) {
          joined = arg
        } else {
          joined += `/${arg}`
        }
      }
    }
    if (joined === undefined) return '.'
    return normalize(joined)
  }

  return { join, baseName }
}

const { join, baseName } = pathModule()

function replacer (k, v) {
  if (typeof v === 'bigint') {
    return v.toString()
  }
  return v
}

function repl () {
  const BUFFER_SIZE = 64 * 1024
  const MAX_BUFFER = 4 * BUFFER_SIZE
  const stdin = new TTY(0)
  const buf = Buffer.alloc(BUFFER_SIZE)
  stdin.setup(buf, UV_TTY_MODE_NORMAL)
  stdin.onRead(len => {
    const source = buf.read(0, len)
    try {
      const result = runScript(source, 'repl')
      const payload = `${JSON.stringify(result, replacer, 2)}\n`
      const r = stdout.write(buf.write(payload, 0))
      if (r < 0) return stdout.close()
    } catch (err) {
      print(err.stack)
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
  stdout.onClose(() => stdin.close())
  if (stdout.write(buf.write('> ', 0)) < 0) {
    stdout.close()
  } else {
    stdin.resume()
  }
}

function readFile (path) {
  const BUFSIZE = 4096
  const buf = Buffer.alloc(BUFSIZE)
  const file = new File()
  const parts = []
  file.setup(buf, buf)
  file.fd = file.open(path, O_RDONLY)
  if (file.fd < 0) {
    throw new Error(`Error opening ${path}: ${loop.error(file.fd)}`)
  }
  file.size = 0
  let len = file.read(BUFSIZE, file.size)
  while (len > 0) {
    file.size += len
    parts.push(buf.read(0, len))
    len = file.read(BUFSIZE, file.size)
  }
  if (len < 0) {
    throw new Error(`Error reading ${path}: ${loop.error(len)}`)
  }
  file.close()
  file.text = parts.join('')
  return file
}

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
    result.push({
      typeName,
      functionName,
      methodName,
      scriptName,
      line,
      column,
      isToplevel,
      isEval,
      isNative,
      isConstructor,
      isWasm,
      isUserJavascript
    })
  }
  try {
    if (Object.getOwnPropertyDescriptor(err, 'frames')) {
      err.frames = result
    } else {
      Object.defineProperty(err, 'frames', {
        value: result,
        writable: true,
        enumerable: true
      })
    }
    if (Object.getOwnPropertyDescriptor(err, 'fileName')) {
      err.fileName = result[0].scriptName
    } else {
      Object.defineProperty(err, 'fileName', {
        value: result[0].scriptName,
        writable: true,
        enumerable: true
      })
    }
    if (Object.getOwnPropertyDescriptor(err, 'lineNumber')) {
      err.lineNumber = result[0].line
    } else {
      Object.defineProperty(err, 'lineNumber', {
        value: result[0].line,
        writable: true,
        enumerable: true
      })
    }
    if (Object.getOwnPropertyDescriptor(err, 'type')) {
      err.type = 'GeneralException'
    } else {
      Object.defineProperty(err, 'type', {
        value: 'GeneralException',
        writable: true,
        enumerable: true
      })
    }
  } catch (e) {}
  return err.stack
}

global.onUncaughtException = err => {
  if (!err) {
    print('onUncaughtException with no Error')
    return
  }
  try {
    Object.defineProperty(err, 'type', {
      value: 'UncaughtException',
      writable: false,
      enumerable: true
    })
  } catch (e) {}
  const stack = err.stack
  if (process.onUncaughtException) return process.onUncaughtException(err)
  print(`${err.type}\n${stack}`)
}

global.onUnhandledRejection = (err, promise) => {
  if (!err) {
    print('onUnhandledRejection with no Error')
    return
  }
  try {
    Object.defineProperty(err, 'promise', {
      value: promise,
      writable: false,
      enumerable: true
    })
    Object.defineProperty(err, 'type', {
      value: 'UnhandledRejection',
      writable: false,
      enumerable: true
    })
  } catch (e) {}
  const stack = err.stack
  if (process.onUnhandledRejection) return process.onUnhandledRejection(err)
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

global.Buffer = {
  empty: () => new GlobalBuffer(),
  alloc: (size, shared = false) => {
    const buf = new GlobalBuffer()
    const ab = shared ? buf.allocShared(size) : buf.alloc(size)
    buf.size = buf.size()
    buf.bytes = ab
    return buf
  },
  fromString: (str, shared = false) => {
    const buf = global.Buffer.alloc(str.length, shared)
    buf.write(str)
    return buf
  },
  fromArrayBuffer: (ab, shared) => {
    const buf = new GlobalBuffer()
    shared ? buf.loadShared(ab) : buf.load(ab)
    buf.size = buf.size()
    buf.bytes = ab
    return buf
  }
}

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
  const usage = _process.heapUsage(heap)
  usage.spaces = Object.keys(usage.heapSpaces).map(k => {
    const space = usage.heapSpaces[k]
    return {
      name: k,
      size: space[2],
      used: space[3],
      available: space[1],
      physicalSize: space[0]
    }
  })
  delete usage.heapSpaces
  return usage
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

process.stats = () => {
  const mem = process.memoryUsage()
  const all = process.activeHandles()
  const handles = all.length
  const active = all.filter(h => h.active).length
  const summary = {}
  all.filter(h => h.active).forEach(h => {
    if (summary[h.type]) return summary[h.type]++
    summary[h.type] = 1
  })
  const cpu = process.cpuUsage()
  const heap = process.heapUsage()
  return {
    threads: Object.keys(threads).length,
    ticks: process.ticks,
    queue: queue.length,
    cpu,
    mem,
    handles,
    active,
    summary,
    heap
  }
}

function activeHandles () {
  const buf = Buffer.alloc(16384)
  loop.listHandles(buf)
  const bytes = new Uint8Array(buf.bytes)
  let off = 0
  const handles = []
  while (1) {
    const active = bytes[off]
    if (active === 255) break
    const len = bytes[off + 1]
    if (len > 0) {
      const type = buf.read(off + 2, len)
      handles.push({ active, type })
    } else {
      handles.push({ active, type: 'unknown' })
    }
    off += (2 + len)
  }
  return handles
}

function nextTick (fn) {
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
  loop.ref()
  idleActive = true
}

function runScriptFromFile (path) {
  const newPath = join(global.__dirname, path)
  const { text } = readFile(newPath)
  global.__dirname = baseName(newPath)
  global.__filename = newPath
  return runScript(text, newPath)
}

function runLoop () {
  do {
    loop.run()
    process.runMicroTasks()
  } while (loop.isAlive())
  process.runMicroTasks()
  if (process.onExit) process.onExit()
}

global.require = (path, parent) => {
  let dirName = parent ? parent.dirName : global.__dirname
  const fileName = join(dirName, path)
  dirName = baseName(fileName)
  let module = cache[fileName]
  if (module) return module.exports
  const params = ['exports', 'require', 'module', 'spawn']
  module = { exports: {}, dirName, fileName }
  const { exports } = module
  module.text = (readFile(fileName)).text
  const fun = compile(module.text, fileName, params, [])
  module.function = fun
  const spawn = (fn, cb = () => {}, opts = { dirName: module.dirName }) => {
    opts.dirName = module.dirName
    return process.spawn(fn, cb, opts)
  }
  fun.call(exports, exports, p => global.require(p, module), module, spawn)
  cache[fileName] = module
  return module.exports
}

global.spawn = (fun, onComplete = () => {}, opts = { ipc: false }) => {
  const { dirName = global.__dirname, ipc, bufSize } = opts
  const thread = new Thread()
  const envJSON = JSON.stringify(process.env, replacer)
  const argsJSON = JSON.stringify(process.args, replacer)
  const envLen = envJSON.length
  const argsLen = argsJSON.length
  const dirNameLen = dirName.length
  const bufferSize = 1 + 4 + 4 + envLen + 4 + argsLen + 4 + dirNameLen + 4
  thread.buffer = Buffer.alloc(bufferSize, true)
  const view = new DataView(thread.buffer.bytes)
  let ipcSize = 0
  thread.view = view
  thread.id = next++
  let fd = 0
  if (ipc) {
    const size = bufSize || process.env.THREAD_BUFFER_SIZE || 4096
    ipcSize = parseInt(size, 10)
    const [rb, wb] = [Buffer.alloc(ipcSize), Buffer.alloc(ipcSize)]
    const sock = new Socket(UNIX)
    const parser = new Parser(rb, wb)
    sock.buffers = { rb, wb }
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
    fd = sock.open()
    if (fd < 0) {
      throw new Error(`Error: ${fd}: ${sock.error(fd)}`)
    }
    thread.sock = sock
  }
  view.setUint8(0, thread.id)
  view.setUint32(1, fd)
  view.setUint32(5, envLen)
  thread.buffer.write(envJSON, 9)
  view.setUint32(envLen + 9, argsLen)
  thread.buffer.write(argsJSON, envLen + 13)
  view.setUint32(envLen + argsLen + 13, dirNameLen)
  thread.buffer.write(dirName, envLen + argsLen + 17)
  view.setUint32(envLen + argsLen + dirNameLen + 17, ipcSize)
  threads[thread.id] = thread
  const r = thread.start(fun, err => {
    delete threads[thread.id]
    onComplete({ err, thread })
  }, thread.buffer)
  if (r !== 0) throw new Error(`Bad Status: ${r} (${loop.error(r)})`)
  return thread
}

global.dv8 = { repl, readFile, join, baseName, cache }
process.activeHandles = activeHandles
process.nextTick = nextTick
process.runLoop = runLoop
process.exec = (...args) => _process.spawn.apply(_process, args)

if (global.workerData) {
  global.workerData.bytes = global.workerData.allocShared()
  const dv = new DataView(global.workerData.bytes)
  process.TID = dv.getUint8(0)
  process.PID = _process.pid()
  process.fd = dv.getUint32(1)
  const envLen = dv.getUint32(5)
  const envJSON = global.workerData.read(9, envLen)
  process.env = JSON.parse(envJSON)
  const argsLen = dv.getUint32(9 + envLen)
  const argsJSON = global.workerData.read(13 + envLen, argsLen)
  process.args = JSON.parse(argsJSON)
  const dirNameLen = dv.getUint32(13 + envLen + argsLen)
  if (dirNameLen > 0) {
    global.__dirname = global.workerData.read(17 + envLen + argsLen, dirNameLen)
  }
  if (process.fd !== 0) {
    const bufSize = dv.getUint32(17 + envLen + argsLen + dirNameLen)
    const [rb, wb] = [Buffer.alloc(bufSize), Buffer.alloc(bufSize)]
    const sock = new Socket(UNIX)
    sock.buffers = { wb, rb }
    const parser = new Parser(rb, wb)
    sock.onConnect(() => {
      sock.setup(rb, wb)
      sock.resume()
    })
    parser.onMessage = message => sock.onMessage(message)
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
  const { workerSource, workerName } = global
  delete global.workerSource
  delete global.workerName
  delete global.workerData
  const newPath = join(global.__dirname, process.args[1])
  const threadName = `${newPath}.#${process.TID}.${workerName || 'anonymous'}`
  const start = workerSource.indexOf('{') + 1
  const end = workerSource.lastIndexOf('}')
  runScript(workerSource.slice(start, end), threadName)
} else {
  process.env = ENV
  process.PID = _process.pid()
  process.TID = 0
  process.args = global.args
  process.threads = threads
  if (process.args.length < 2) {
    repl()
  } else {
    if (process.args[1] === '-e') {
      runScript(global.args[2], 'eval')
    } else {
      runScriptFromFile(process.args[1])
    }
  }
}

runLoop()
