require('./base.js')
const { Thread } = module('thread', {})

const threads = []

function spawn() {
    const now = Date.now()
    const threadId = threads.length
    print(`main  : ${threadId} starting`)
    const thread = new Thread()
    thread.buffer = new Buffer()
    const dv = new DataView(thread.buffer.alloc(256))
    dv.setUint8(0, threadId) // thread id
    dv.setUint8(1, parseInt(args[2] || '10', 10)) // time to run
    thread.start('./thread-worker.js', () => {
        print(`main  : ${threadId} complete`)
        timer.stop()
    }, thread.buffer)
    threads.push(thread)
    const timer = setInterval(() => {
        const id = dv.getUint8(0)
        const duration = dv.getUint8(1)
        const time = dv.getFloat64(2)
        const ready = dv.getFloat64(10)
        print(JSON.stringify({ id, duration, time, ready, boot: ready - now, date: new Date(time) }))
    }, 1000)
}

let count = 10
while(count--) spawn()
