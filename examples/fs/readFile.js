const { File, O_RDONLY } = module('fs', {})

let len = 0
const BUFSIZE = 65536

const buf = Buffer.alloc(BUFSIZE)
const fs = new File()
fs.setup(buf, buf)

fs.open(process.args[2] || './fs.js', O_RDONLY)
len = fs.read(BUFSIZE)
if (len > 0) print(buf.read(0, len))
fs.close()

fs.open(process.args[2] || './readFile.js', O_RDONLY)
len = fs.read(BUFSIZE)
if (len > 0) print(buf.read(0, len))
fs.close()

fs.open(process.args[2] || './foo.json', O_RDONLY)
len = fs.read(BUFSIZE)
if (len > 0) print(buf.read(0, len))
fs.close()

print(JSON.stringify(JSON.parse(buf.read(0, len)), null, '  '))
