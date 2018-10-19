require('./lib/base.js')
const { Thread } = module('thread', {})
const { createBuffer } = require('./lib/util.js')

const BUFFER_SIZE = 64 * 1024
const threads = []

function spawn() {
    const start = hrtime()
    const threadId = threads.length + 1
    const thread = new Thread()
    thread.buffer = createBuffer(BUFFER_SIZE)
    const dv = new DataView(thread.buffer.bytes)
    const duration = parseInt(args[3] || '10', 10)
    dv.setUint8(0, threadId)
    dv.setUint8(1, duration)
    thread.start('./thread-worker.js', () => {
        const ready = dv.getBigUint64(10)
        const i = BigInt(1000)
        print(`boot: ${(ready - start) / i} usec`)
    }, thread.buffer)
    threads.push(thread)
}

let count = parseInt(args[2] || '4', 10)
while(count--) spawn()
