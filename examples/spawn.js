const { Thread } = module('thread', {})

let id = 1

onExit(() => {
    print('exiting main')
})

function spawn(fname) {
    const thread = new Thread()
    thread.buffer = new Buffer()
    const bytes = new Uint8Array(thread.buffer.alloc(100))
    const threadId = bytes[0] = id++
    thread.start(fname, () => {
        print(`worker ${threadId} stopped`)
    }, thread.buffer)
}

const WORKERS = parseInt(args[3] || '4', 10)
let i = WORKERS
while(i--) {
    spawn(args[2])
}
