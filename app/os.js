const global = this

const { OS } = module('os', {})
const os = new OS()

async function run() {
    print(`sleep for ${seconds} seconds`)
    os.sleep(seconds)
    print(`Date: ${new Date()}`)
    shutdown()
}

run().catch(err => print(`error:\n${err.toString()}`))