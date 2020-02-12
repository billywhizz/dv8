const { library, path } = dv8
const { join } = path
const { Socket, PIPE } = library('socket')
const { Process } = library('process')
const { File, FileSystem } = library('fs', {})
const {
  O_CREAT,
  O_TRUNC,
  O_RDONLY,
  O_WRONLY,
  S_IRUSR,
  S_IWUSR,
  S_IXUSR,
  S_IRGRP,
  S_IWGRP,
  S_IXGRP,
  S_IROTH,
  S_IWOTH,
  S_IXOTH,
  S_IFMT, // bit mask for the file type bit field
  S_IFSOCK, // socket
  S_IFLNK, // symbolic link
  S_IFREG, // regular file
  S_IFBLK, // block device
  S_IFDIR, // directory
  S_IFCHR, // character device
  S_IFIFO, // FIFO
  DT_BLK,
  DT_CHR,
  DT_DIR,
  DT_FIFO,
  DT_LNK,
  DT_REG,
  DT_SOCK,
  DT_UNKNOWN
} = FileSystem

const { fstat } = FileSystem
const statArray = new BigUint64Array(32)
const st = {}
const MAX_DIR_SIZE = 1024
const listing = new Array(MAX_DIR_SIZE)

function exec (child, buf = Buffer.alloc(4096)) {
  const stdin = new Socket(PIPE)
  const stdout = new Socket(PIPE)
  const stderr = new Socket(PIPE)
  const process = new Process()
  stdin.create()
  stdin.setup(buf, buf)
  stdout.create()
  stdout.setup(buf, buf)
  stderr.create()
  stderr.setup(buf, buf)
  const { file, args, cwd = Process.cwd() } = child
  child.pid = process.spawn(file, cwd, args.map(v => v.toString()), stdin, stdout, stderr, (status, signal) => {
    child.status = Number(status.toString())
    child.signal = signal
    if (child.onExit) child.onExit(status, signal)
  })
  child.stdin = stdin
  child.stdout = stdout
  child.stderr = stderr
  child.buffer = buf
  return child
}

function checkFlag (val, flag) {
  return (val & flag) === flag
}

function checkMode (val, mode) {
  return (val & S_IFMT) === mode
}

function fileType (type) {
  if (type === DT_BLK) return 'block'
  if (type === DT_CHR) return 'character'
  if (type === DT_DIR) return 'directory'
  if (type === DT_FIFO) return 'fifo'
  if (type === DT_LNK) return 'symlink'
  if (type === DT_REG) return 'regular'
  if (type === DT_SOCK) return 'socket'
  return 'unknown'
}

function setStats (fileName) {
  st.fileName = fileName
  st.deviceId = statArray[0]
  st.mode = Number(statArray[1])
  st.hardLinks = statArray[2]
  st.uid = statArray[3]
  st.gid = statArray[4]
  st.rdev = statArray[5] // ?
  st.inode = statArray[6]
  st.size = statArray[7]
  st.blockSize = statArray[8]
  st.blocks = statArray[9]
  st.flags = statArray[10]
  st.st_gen = statArray[11] // ?
  st.accessed = { tv_sec: statArray[12], tv_usec: statArray[13] }
  st.modified = { tv_sec: statArray[14], tv_usec: statArray[15] }
  st.created = { tv_sec: statArray[16], tv_usec: statArray[17] }
  st.permissions = {
    user: { r: checkFlag(st.mode, S_IRUSR), w: checkFlag(st.mode, S_IWUSR), x: checkFlag(st.mode, S_IXUSR) },
    group: { r: checkFlag(st.mode, S_IRGRP), w: checkFlag(st.mode, S_IWGRP), x: checkFlag(st.mode, S_IXGRP) },
    other: { r: checkFlag(st.mode, S_IROTH), w: checkFlag(st.mode, S_IWOTH), x: checkFlag(st.mode, S_IXOTH) }
  }
  st.type = {
    socket: checkMode(st.mode, S_IFSOCK),
    symlink: checkMode(st.mode, S_IFLNK),
    regular: checkMode(st.mode, S_IFREG),
    block: checkMode(st.mode, S_IFBLK),
    directory: checkMode(st.mode, S_IFDIR),
    character: checkMode(st.mode, S_IFCHR),
    fifo: checkMode(st.mode, S_IFIFO)
  }
}

function stat (path, flags = O_RDONLY) {
  const file = new File()
  path = join(module.dirName, path)
  file.fd = file.open(path, flags)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  FileSystem.fstat(file, statArray)
  setStats(path)
  file.close()
  return st
}

function readFile (path, flags = O_RDONLY) {
  const file = new File()
  path = join(module.dirName, path)
  file.fd = file.open(path, flags)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  fstat(file, statArray)
  setStats(path)
  file.size = Number(st.size)
  const buf = Buffer.alloc(file.size)
  file.setup(buf, buf)
  const len = file.read(file.size, 0)
  if (len < 0) throw new Error(`Error reading ${path}: ${len}`)
  file.close()
  return buf
}

function writeFile (path, buf, flags = O_CREAT | O_TRUNC | O_WRONLY, mode = S_IRUSR | S_IWUSR) {
  const file = new File()
  file.setup(buf, buf)
  path = join(module.dirName, path)
  file.fd = file.open(path, flags, mode)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  file.write(buf.size, 0)
  file.close()
}

const lookup = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'.split('')

const buf2b64 = (buf, len = buf.size) => {
  const bytes = new Uint8Array(buf.bytes.slice(0, len))
  let i = 0
  const encoded = []
  while (i < len) {
    const a = i < len ? bytes[i++] : 0
    const b = i < len ? bytes[i++] : 0
    const c = i < len ? bytes[i++] : 0
    const triple = (a << 0x10) + (b << 0x08) + c
    encoded.push(lookup[(triple >> 3 * 6) & 0x3f])
    encoded.push(lookup[(triple >> 2 * 6) & 0x3f])
    encoded.push(lookup[(triple >> 1 * 6) & 0x3f])
    encoded.push(lookup[(triple >> 0 * 6) & 0x3f])
  }
  const over = len % 3
  if (over === 1) {
    encoded[encoded.length - 1] = '='
    encoded[encoded.length - 2] = '='
  } else if (over === 2) {
    encoded[encoded.length - 1] = '='
  }
  return encoded.join('')
}

const rx = /\B(?=(\d{3})+(?!\d))/g

function formatRPS (count, ms) {
  return (Math.floor((count / (ms / 1000)) * 100) / 100).toFixed(2).replace(rx, ',')
}

const readdir = path => {
  path = join(module.dirName, path)
  return listing.slice(0, FileSystem.readdir(path, listing))
}
const stringify = (o, sp = '  ') => JSON.stringify(o, (k, v) => (typeof v === 'bigint') ? Number(v) : v, sp)

const defaultIgnore = ['length', 'name', 'arguments', 'caller', 'constructor']

const ANSI_RED = '\u001b[31m'
const ANSI_MAGENTA = '\u001b[35m'
const ANSI_DEFAULT = '\u001b[0m'
const ANSI_CYAN = '\u001b[36m'
const ANSI_GREEN = '\u001b[32m'
const ANSI_WHITE = '\u001b[37m'
const ANSI_YELLOW = '\u001b[33m'

/* eslint-disable */
String.prototype.magenta = function (pad) { return `${ANSI_MAGENTA}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.red = function (pad) { return `${ANSI_RED}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.green = function (pad) { return `${ANSI_GREEN}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.cyan = function (pad) { return `${ANSI_CYAN}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.white = function (pad) { return `${ANSI_WHITE}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.yellow = function (pad) { return `${ANSI_YELLOW}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
/* eslint-enable */

function inspect (o, indent = 0, max = 100, ignore = defaultIgnore, lines = [], colors = 'off') {
  if (indent === max) return lines
  try {
    if (typeof o === 'object') {
      const props = Object.getOwnPropertyNames(o).filter(n => (ignore.indexOf(n) === -1))
      const hasConstructor = o.hasOwnProperty('constructor') // eslint-disable-line
      props.forEach(p => {
        if (p === 'global' || p === 'globalThis') return
        if (typeof o[p] === 'object') {
          lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').cyan(0)}`, false)
        } else if (typeof o[p] === 'function') {
          if (hasConstructor) {
            lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').yellow(0)}`, false)
          } else {
            lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').magenta(0)}`, false)
          }
        } else {
          lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').green(0)}`, false)
        }
        inspect(o[p], indent + 1, max, ignore, lines)
      })
    } else if (typeof o === 'function') {
      const props = Object.getOwnPropertyNames(o).filter(n => (ignore.indexOf(n) === -1))
      const hasConstructor = o.hasOwnProperty('constructor') // eslint-disable-line
      props.forEach(p => {
        if (p === 'prototype') {
          inspect(o[p], indent + 1, max, ignore, lines)
        } else {
          if (typeof o[p] === 'object') {
            lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').cyan(0)}`, false)
          } else if (typeof o[p] === 'function') {
            if (hasConstructor) {
              lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').yellow(0)}`, false)
            } else {
              lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').magenta(0)}`, false)
            }
          } else {
            lines.push(`\n${p.padStart(p.length + (indent * 2), ' ').green(0)}`, false)
          }
          inspect(o[p], indent + 1, max, ignore, lines)
        }
      })
    } else {
      lines.push(`: ${o.toString()}`, false)
    }
  } catch (err) {}
  return lines
}

const statCache = {}

function watchFile (path, onChange, ms = 1000) {
  const file = new File()
  file.fd = file.open(path, O_RDONLY)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  file.timer = setInterval(() => {
    FileSystem.fstat(file, statArray)
    setStats(path)
    const last = statCache[path]
    if (!last) {
      statCache[path] = Object.assign({}, st)
      return
    }
    if (st.modified.tv_sec > last.modified.tv_sec) {
      onChange(st)
      statCache[path] = Object.assign({}, st)
    }
  }, ms)
  return file
}

function buf2hex (ab, len) {
  return Array.prototype.map.call((new Uint8Array(ab)).slice(0, len), x => ('00' + x.toString(16)).slice(-2)).join('')
}

function mkdir (path) {
  path = join(module.dirName, path)
  return FileSystem.mkdir(path)
}

const commas = str => str.toString().replace(rx, ',')

module.exports = { mkdir, readFile, writeFile, stat, FileSystem, exec, readdir, fileType, formatRPS, stringify, inspect, commas, watchFile, buf2b64, buf2hex }
