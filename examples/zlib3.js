const { ZLib, ZLIB_MODE_GZIP, ZLIB_MODE_GUNZIP, Z_FINISH } = module('libz', {})
const { buf2hex } = require('./lib/util.js')
const BUFSIZE = 1024
const [ rb, wb ] = [ Buffer.alloc(BUFSIZE), Buffer.alloc(BUFSIZE) ]
const value = process.args[2] || 'hello'
const compression = parseInt(process.args[3] || 9)
const deflate = new ZLib(ZLIB_MODE_GZIP)
deflate.setup(wb, rb, compression)
let size = BUFSIZE - deflate.write(wb.write(value), Z_FINISH)
deflate.end()
const out = Buffer.alloc(BUFSIZE)
const inflate = new ZLib(ZLIB_MODE_GUNZIP)
inflate.setup(rb, out, compression)
size = BUFSIZE - inflate.write(size, Z_FINISH)
inflate.end()
print(buf2hex(out, size))
print(out.read(0, size))
