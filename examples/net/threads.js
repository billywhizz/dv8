onExit(() => print('exiting main'))
const WORKERS = parseInt(args[2] || '4', 10)
let i = WORKERS
const threads = {}

function dumpError(err) {
    print(
`line       : ${err.line}
filename    : ${err.filename}
exception   : ${err.exception}
sourceline  : ${err.sourceline}
stack       : ${err.stack}
`
    )
}

while(i--) {
    const thread = createThread(() => {
        const { bytes } = workerData
        const dv = new DataView(bytes)
        let i = 50000000n
        while(i--) {
            dv.setBigUint64(2048, i)
        }
        //print(JSON.stringify(args))
        //print(JSON.stringify(env))
        //print(JSON.stringify(memoryUsage()))
    }, ({ err, thread, status }) => {
        print(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}, status: ${status}`)
        if (err) dumpError(err)
        delete threads[thread.id]
    })
    threads[thread.id] = thread
}

const timer = setInterval(() => {
    const ids = Object.keys(threads).map(Number)
    if (!ids.length) return clearTimeout(timer)
    ids.sort((a, b) => a > b ? 1 : ( a < b ? -1 : 0))
    print(ids.map(id => `\u001b[36m${threads[id].id}\u001b[0m: ${threads[id].dv.getBigUint64(2048)}`).join(' : '))
}, 1000)