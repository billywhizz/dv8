const { File, O_RDWR, O_CREAT, O_TRUNC } = module('fs', {})
const { ZLib, ZLIB_MODE_GZIP, ZLIB_MODE_GUNZIP, Z_FINISH, Z_NO_FLUSH, Z_FULL_FLUSH } = module('libz', {})
const { buf2hex } = require('./lib/util.js')

const BUFSIZE = 1024

const [ rb, wb ] = [ Buffer.alloc(BUFSIZE), Buffer.alloc(BUFSIZE) ]
const value = process.args[2] || 'hello'
const compression = parseInt(process.args[3] || 9)
const zlib = new ZLib(ZLIB_MODE_GZIP)

let r = zlib.setup(wb, rb, compression)
r = zlib.write(wb.write(value), Z_FINISH)
if (r < 0) throw new Error('Bad Write')
print(`done: ${BUFSIZE - r}`)
const size = BUFSIZE - r
r = zlib.end()
print(`end: ${r}`)
if (r !== 0) throw new Error('Error Finalizing')
print(buf2hex(rb, size))

const fs = new File()
r = fs.setup(rb, rb)
print(`setup: ${r}`)
const fd = fs.open('./hello.gz', O_RDWR | O_CREAT | O_TRUNC)
print(`open: ${fd}`)
r = fs.write(size)
print(`write: ${r}`)
r = fs.close()
print(`close: ${r}`)

const wb2 = Buffer.alloc(BUFSIZE)
const zlib2 = new ZLib(ZLIB_MODE_GUNZIP)
r = zlib2.setup(rb, wb2, compression)
r = zlib2.write(size, Z_FINISH)
if (r < 0) throw new Error('Bad Write')
print(`done: ${BUFSIZE - r}`)
const size2 = BUFSIZE - r
r = zlib2.end()
print(`end: ${r}`)
if (r !== 0) throw new Error('Error Finalizing')
print(buf2hex(wb2, size2))
print(wb2.read(0, size))
