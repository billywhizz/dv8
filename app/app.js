const { OS } = module('os', {})
const { Process } = module('process', {})
const { Socket } = module('socket', {})

async function run() {
    const seconds = 5
    const os = new OS()
    const process = new Process()
    print(`Version: ${version()}`)
    print(`PID: ${process.pid()}`)
    print(`Date: ${new Date()}`)
    print(`sleep for ${seconds} seconds`)
    os.sleep(seconds)
    print(`Date: ${new Date()}`)
    const memValues = new Float64Array(4)
    process.memoryUsage(memValues)
    const mem = {
        rss: memValues[0],
        heapTotal: memValues[1],
        heapUsed: memValues[2],
        external: memValues[3]
    }
    print(JSON.stringify(mem, null, '  '))
    const sock = new Socket(0)
    print(`listen: ${sock.listen('0.0.0.0', 3000)}`)
    //os.sleep(120)
}

run().catch(console.error)
