const { Process } = module('process', {})

const process = new Process()

const mem = new Float64Array(4)

function getMemoryUsage() {
    process.memoryUsage(mem)
    return {
        rss: mem[0],
        heapTotal: mem[1],
        heapUsed: mem[2],
        external: mem[3]
    }
}

const buffers = []

const b = new ArrayBuffer(0xffffffff)
print(b.byteLength)


function newBuffer() {
    const b = new Buffer()
    const size = Math.pow(2, 31) - 1
    const ab = b.alloc(size)
    buffers.push(b)
}

const t = new Timer()
t.start(() => {
    print(JSON.stringify(getMemoryUsage(), null, '  '))
    newBuffer()
}, 1000, 1000)

