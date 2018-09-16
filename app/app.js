const { OS } = module({})

async function run() {
    const seconds = 5
    const os = new OS()
    print(`Version: ${version()}`)
    print(`Date: ${new Date()}`)
    print(`sleep for ${seconds} seconds`)
    os.sleep(seconds)
    print(`Date: ${new Date()}`)

}

run().catch(console.error)
