onExit(() => {
    print('main is exiting')
})

global.onUncaughtException = err => {
    print('OMG!!!')
}

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

const t1 = createThread(() => {
    const timer = setInterval(() => {
        print(`thread ${threadId} running`)
    }, 1000)
    setTimeout(() => clearTimeout(timer), 10000)
}, ({ err, thread, status }) => {
    print(`thread: ${thread.id}\nstatus: ${status}\ntime: ${thread.time}\nboot: ${thread.boot}`)
    if (err) dumpError(err)
})

const t2 = createThread(() => {
    const timer = setInterval(() => {
        print(`thread ${threadId} running`)
    }, 1000)
    setTimeout(() => clearTimeout(timer), 10000)
}, ({ err, thread, status }) => {
    print(`thread: ${thread.id}\nstatus: ${status}\ntime: ${thread.time}\nboot: ${thread.boot}`)
    if (err) dumpError(err)
})

function spawn(fun) {
    return new Promise(ok => createThread(fun, ok))
}

function threadFunc() {
    // this all runs on a separate thread
    global.onUncaughtException = err => {
        print('OMG!!!')
        print(err.message)
        print(err.stack)
    }
    onExit(() => {
        print('thread is exiting')
    })
    const timer = setInterval(() => {
        print(`thread ${threadId} running`)
    }, 1000)
    setTimeout(() => clearTimeout(timer), 3000)
    setTimeout(() => {
        throw new Error('Foo')
    }, 5000)
    const dv = new DataView(global.workerData.bytes)
    dv.setBigUint64(0, BigInt(hrtime()))
}

async function run() {
    const { err, thread, status } = await spawn(threadFunc)
    print(`thread: ${thread.id}\nstatus: ${status}\ntime: ${thread.time}\nboot: ${thread.boot}`)
    if (err) dumpError(err)
}

run().catch(console.error)