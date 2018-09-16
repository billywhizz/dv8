const { OS } = module('os', {})
const { Process } = module('process', {})

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
}

run().catch(console.error)
