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

//const n = 2n ** 32n
//const b = new ArrayBuffer(n)


const b = new Buffer()
//const ab = b.alloc(Math.pow(2,30))
const size = Math.pow(2, 31) - 1
//const size = 10
print(`size: ${size}`)
const ab = b.alloc(size)
if (ab) {
    print(`ArrayBuffer: ${ab.byteLength}`)
    const dv = new Uint8Array(ab)
    let i = ab.byteLength
    let bb
    while(i--) {
        dv[i] = i & 0xff;
    }
    i = ab.byteLength
    while(i--) {
        if (dv[i] !== (i & 0xff)) {
            print('bad')
        }
    }
}
print(`${b.length()}`)

const t = new Timer()
t.start(() => {
    print(JSON.stringify(getMemoryUsage(), null, '  '))
}, 1000, 1000)

