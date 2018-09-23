const global = this

const { OS } = module('os', {})
const { Process } = module('process', {})
const { Socket } = module('socket', {})

const process = new Process()
const os = new OS()
const sock = new Socket(0)

const seconds = 1

function getMemoryUsage() {
    const memValues = new Float64Array(4)
    process.memoryUsage(memValues)
    return {
        rss: memValues[0],
        heapTotal: memValues[1],
        heapUsed: memValues[2],
        external: memValues[3]
    }
}

async function run() {
    print(`Version: ${version()}`)
    print(`PID: ${process.pid()}`)
    print(`Date: ${new Date()}`)
    print(`sleep for ${seconds} seconds`)
    os.sleep(seconds)
    print(`Date: ${new Date()}`)
    print(JSON.stringify(getMemoryUsage(), null, '  '))
    print(`listen: ${sock.listen('0.0.0.0', 3000)}`)
    print('done')
    const b = new Buffer()
    print(`b:\n${JSON.stringify(Object.getOwnPropertyNames(b))}`)
    const ab = b.alloc(100)
    print(`ArrayBuffer:\n  Byte Length: ${ab.byteLength}`)
    shutdown()
}

run().catch(err => print(`error:\n${err.toString()}`))
print(`this:\n${JSON.stringify(Object.getOwnPropertyNames(global))}`)
print(`console:\n${JSON.stringify(Object.getOwnPropertyNames(console))}`)
print(`global:\n${JSON.stringify(Object.getOwnPropertyNames(global))}`)
print(`Buffer:\n${JSON.stringify(Object.getOwnPropertyNames(Buffer))}`)
