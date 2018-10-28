onExit(() => print('exiting main'))
const WORKERS = parseInt(args[2] || '4', 10)
let i = WORKERS
const threads = {}
while(i--) {
    const thread = createThread(() => {
        const { bytes } = workerData
        const dv = new DataView(bytes)
        let i = 5000000n
        while(i--) {
            dv.setBigUint64(100, i)
        }
        print(JSON.stringify(args))
        print(JSON.stringify(env))
        print(JSON.stringify(memoryUsage()))
    }, thread => {
        print(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}`)
        delete threads[thread.id]
    })
    threads[thread.id] = thread
}

const timer = setInterval(() => {
    const ids = Object.keys(threads).map(Number)
    if (!ids.length) return clearTimeout(timer)
    ids.sort((a, b) => a > b ? 1 : ( a < b ? -1 : 0))
    print(ids.map(id => `\u001b[36m${threads[id].id}\u001b[0m: ${threads[id].dv.getBigUint64(100)}`).join(' : '))
}, 1000)