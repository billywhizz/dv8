const { library } = dv8
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
const statArray = new BigUint64Array(16)
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
  file.fd = file.open(path, flags)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  FileSystem.fstat(file, statArray)
  setStats(path)
  return st
}

function readFile (path, flags = O_RDONLY) {
  const file = new File()
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
  file.fd = file.open(path, flags, mode)
  if (file.fd < 0) throw new Error(`Error opening ${path}: ${file.fd}`)
  file.write(buf.size, 0)
  file.close()
}

const readdir = path => listing.slice(0, FileSystem.readdir(path, listing))

module.exports = { readFile, writeFile, stat, FileSystem, exec, readdir, fileType }
