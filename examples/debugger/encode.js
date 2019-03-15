const { buf2b64 } = require('../lib/util.js')

const buf = Buffer.alloc(1024)
const bytes = new Uint8Array(buf.bytes)

bytes[0] = 1
bytes[1] = 2
bytes[2] = 3
bytes[3] = 4
bytes[4] = 5
bytes[5] = 255
bytes[6] = 254
bytes[7] = 253
bytes[8] = 252
bytes[9] = 64
bytes[10] = 65
bytes[11] = 66

print(buf2b64(buf, 10))
print(buf2b64(buf, 11))
print(buf2b64(buf, 12))
