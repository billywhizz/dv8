require('./lib/base.js')
const { Thread } = module('thread', {})

let id = 1

onExit(() => {
    print('exiting main')
})

function spawn(fname) {
    const start = Date.now()
    const thread = new Thread()
    thread.buffer = new Buffer()
    const dv = new DataView(thread.buffer.alloc(256))
    const threadId = id++
    dv.setUint8(0, threadId)
    thread.start(fname, () => {
        const finish = Date.now()
        print(`worker ${threadId} stopped. time: ${finish - start} ms`)
    }, thread.buffer)
}

const WORKERS = parseInt(args[3] || '4', 10)
let i = WORKERS
while(i--) {
    spawn(args[2])
}
