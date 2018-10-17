require('./lib/base.js')
const { Thread } = module('thread', {})

const threads = []

function spawn() {
    const start = hrtime()
    const threadId = threads.length + 1
    print(`main  : ${threadId} starting`)
    const thread = new Thread()
    thread.buffer = new Buffer()
    const dv = new DataView(thread.buffer.alloc(256))
    const duration = parseInt(args[3] || '10', 10)
    dv.setUint8(0, threadId) // thread id
    dv.setUint8(1, duration) // time to run
    thread.start('./thread-worker.js', () => {
        const ready = dv.getBigUint64(10)
        print(`boot: ${(ready - start) / 1000n} usec`)
    }, thread.buffer)
    threads.push(thread)
}

let count = parseInt(args[2] || '4', 10)
while(count--) spawn()
