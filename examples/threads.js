require('./base.js')
const { Thread } = module('thread', {})

const threads = []

function spawn() {
    const threadId = threads.length
    print(`main  : ${threadId} starting`)
    const thread = new Thread()
    thread.buffer = new Buffer()
    const bytes = new Uint8Array(thread.buffer.alloc(10))
    bytes[0] = threadId // max 255
    bytes[1] = 10 // time to run
    thread.start(thread.buffer, () => {
        print(`main  : ${threadId} complete`)
        timer.stop()
    })
    threads.push(thread)
    const timer = setInterval(() => {
        const [ id, duration, t1, t2, t3, t4 ] = bytes
        const time = (t1 << 24) + (t2 << 16) + (t3 << 8) + t4
        print(JSON.stringify({ id, duration, time, date: new Date(time * 1000) }))
    }, 1000)
}

spawn()
spawn()