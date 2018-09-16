async function run() {
    print(`Version: ${version()}`)
    const seconds = 5
    print(`sleeping for ${seconds} seconds`)
    sleep(seconds)
    print('done sleeping')
    const now = new Date()
    print(`Date: ${now}`)
}

run().catch(console.error)
