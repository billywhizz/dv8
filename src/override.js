// functions

// Buffer methods
function alloc (size, shared) {
  const buf = Buffer.create()
  buf.bytes = shared ? buf.allocShared(size) : buf.alloc(size)
  buf.size = buf.size()
  return buf
}

function fromString (str, shared) {
  const buf = Buffer.alloc(str.length, shared)
  buf.write(str)
  return buf
}

function fromArrayBuffer (ab, shared) {
  const buf = Buffer.create()
  shared ? buf.loadShared(ab) : buf.load(ab)
  buf.size = buf.size()
  buf.bytes = ab
  return buf
}

// Global Timer functions
function setInterval (fn, repeat) {
  const timer = new (dv8.library('timer').Timer)()
  timer.start(fn, repeat, repeat)
  return timer
}

function setTimeout (fn, repeat) {
  const timer = new (dv8.library('timer').Timer)()
  timer.start(fn, repeat)
  return timer
}

function clearTimeout (timer) {
  timer.stop()
  timer.close()
}

// Wrappers for C++ builtins
function wrapLibrary (library) {
  return name => {
    if (!dv8[name]) {
      dv8[name] = {}
      library(name, dv8[name])
    }
    return dv8[name]
  }
}

function wrapHrtime (hrtime) {
  const time = new BigUint64Array(1)
  return () => {
    hrtime(time)
    return time[0]
  }
}

function wrapCpuUsage (cpuUsage) {
  const cpu = new Float64Array(2)
  return () => {
    cpuUsage(cpu)
    return {
      user: cpu[0],
      system: cpu[1]
    }
  }
}

function wrapHeapUsage (heapUsage) {
  const heap = Array.from(new Array(16)).map(v => new Float64Array(4))
  return () => {
    const usage = heapUsage(heap)
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
}

function wrapMemoryUsage (memoryUsage) {
  const mem = new Float64Array(16)
  return () => {
    memoryUsage(mem)
    return {
      rss: mem[0],
      total_heap_size: mem[1],
      used_heap_size: mem[2],
      external_memory: mem[3],
      heap_size_limit: mem[5],
      total_available_size: mem[10],
      total_heap_size_executable: mem[11],
      total_physical_size: mem[12]
    }
  }
}

function wrapEnv (env) {
  return () => {
    return env()
      .map(entry => entry.split('='))
      .reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})
  }
}

// read a file as text
function readFile (path) {
  const { File, O_RDONLY } = dv8.library('fs')
  const file = new File()
  const BUFSIZE = 4096
  const buf = Buffer.alloc(BUFSIZE)
  const parts = []

  file.setup(buf, buf)
  file.fd = file.open(path, O_RDONLY)
  if (file.fd < 0) {
    throw new Error(`Error opening ${path}: ${file.fd}`)
  }
  file.size = 0
  let len = file.read(BUFSIZE, file.size)
  while (len > 0) {
    file.size += len
    parts.push(buf.read(0, len))
    len = file.read(BUFSIZE, file.size)
  }
  if (len < 0) {
    throw new Error(`Error reading ${path}: ${len}`)
  }
  file.close()
  file.text = parts.join('')

  return file
}

// Local Modules

// Path functions for require (lifted from node.js!! =P)
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

// require Module
function requireModule () {
  let pathMod
  const cache = {}
  let process
  const { compile, library } = dv8

  function require (path, parent) {
    if (!process) process = new (library('process').Process)()
    if (!pathMod) pathMod = pathModule()
    const { join, baseName } = pathMod
    let dirName = parent ? parent.dirName : dv8.process.Process.cwd()
    const fileName = join(dirName, path)
    dirName = baseName(fileName)
    let module = cache[fileName]
    if (module) return module.exports
    const params = ['exports', 'require', 'module', 'spawn']
    const exports = {}
    module = { exports, dirName, fileName }
    module.text = (readFile(fileName)).text
    dv8.print(fileName)
    const fun = compile(module.text, fileName, params, [])
    module.function = fun
    const spawn = (fn, cb = () => {}, opts = { dirName: module.dirName }) => {
      opts.dirName = module.dirName
      return process.spawn(fn, cb, opts)
    }
    fun.call(exports, exports, p => dv8.require(p, module), module, spawn)
    cache[fileName] = module
    return module.exports
  }

  return { require, cache }
}

// repl Module
function replModule () {
  let stdin
  let stdout
  const { runScript, print, library } = dv8

  function repl () {
    if (!stdin) stdin = new (library('tty').TTY)(0)
    if (!stdout) stdout = new (library('tty').TTY)(1)
    const { UV_TTY_MODE_NORMAL } = dv8.tty
    const BUFFER_SIZE = 64 * 1024
    const MAX_BUFFER = 4 * BUFFER_SIZE
    const buf = Buffer.alloc(BUFFER_SIZE)
    stdin.setup(buf, UV_TTY_MODE_NORMAL)
    stdin.onRead(len => {
      const source = buf.read(0, len)
      try {
        const result = runScript(source, 'repl')
        const payload = `${JSON.stringify(result, null, 2)}\n`
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
    stdout.setup(buf, UV_TTY_MODE_NORMAL)
    stdout.onDrain(() => stdin.resume())
    stdout.onClose(() => stdin.close())
    if (stdout.write(buf.write('> ', 0)) < 0) {
      stdout.close()
    } else {
      stdin.resume()
    }
  }

  return { repl }
}

// main entrypoint
function main (args) {
  dv8.library = wrapLibrary(dv8.library)
  const { workerSource, workerName, runScript } = dv8
  const { require } = requireModule()
  const { repl } = replModule()
  const { Process } = dv8.library('process')
  const { EventLoop } = dv8.library('loop')
  const { pid, cpuUsage, hrtime, heapUsage, runMicroTasks } = Process

  Error.stackTraceLimit = 1000
  Buffer.alloc = alloc
  Buffer.fromString = fromString
  Buffer.fromArrayBuffer = fromArrayBuffer

  dv8.pid = () => pid()
  dv8.env = wrapEnv(dv8.env)
  dv8.memoryUsage = wrapMemoryUsage(dv8.memoryUsage)
  dv8.cpuUsage = wrapCpuUsage(cpuUsage)
  dv8.hrtime = wrapHrtime(hrtime)
  dv8.heapUsage = wrapHeapUsage(heapUsage)
  dv8.require = require
  dv8.repl = repl
  dv8.runMicroTasks = runMicroTasks
  dv8.readFile = readFile

  global.setTimeout = setTimeout
  global.clearTimeout = clearTimeout
  global.setInterval = setInterval

  delete global.eval // eslint-disable-line
  delete global.WebAssembly
  delete global.send // todo - we need this for inspector
  delete global.console

  if (workerSource) {
    const start = workerSource.indexOf('{') + 1
    const end = workerSource.lastIndexOf('}')
    delete dv8.workerSource
    delete dv8.workerName
    runScript(workerSource.slice(start, end), workerName)
  } else if (args.length > 1) {
    if (args[1] === '-e' && args.length > 2) {
      runScript(args[2], 'eval')
    } else {
      runScript(readFile(args[1]).text, args[1]) // todo: check valid path name - stat file first?
    }
  } else {
    repl()
  }
  const loop = new EventLoop()
  loop.run()
}

main(dv8.args)
