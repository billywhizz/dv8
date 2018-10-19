require('./lib/base.js')
const { Thread } = module('thread', {})

const BUFFER_SIZE = 64 * 1024
let threadId = 1

onExit(() => print('exiting main'))

function spawn(fname) {
    const start = Date.now()
    const thread = new Thread()
    thread.buffer = createBuffer(BUFFER_SIZE)
    const dv = new DataView(thread.buffer.bytes)
    dv.setUint8(0, threadId++)
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
