require('./base.js')
const { Thread } = module('thread', {})

let id = 1

function spawn(fname) {
    const thread = new Thread()
    thread.buffer = new Buffer()
    const bytes = new Uint8Array(thread.buffer.alloc(10))
    const threadId = bytes[0] = id++
    thread.start(fname, () => {
        print(`worker ${threadId} stopped`)
    }, thread.buffer)
}

async function run() {
    const WORKERS = 4
    let i = WORKERS
    while(i--) {
        spawn('./server.js')
    }
}

run().catch(console.error)
