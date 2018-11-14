function dumpError(err) {
    print(
`line       : ${err.line}
code        : ${err.code}
message     : ${err.message}
filename    : ${err.filename}
exception   : ${err.exception}
sourceline  : ${err.sourceline}
stack       : ${err.stack}
`
    )
}

onExit(() => print('exiting main'))
global.onUncaughtException = dumpError
onUnhandledRejection(dumpError)

const threads = {}
const WORKERS = parseInt(args[3] || '1', 10)
let i = WORKERS
while(i--) {
    const thread = createThread(args[2], ({ err, thread, status }) => {
        print(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}, status: ${status}`)
        if (err) dumpError(err)
        delete threads[thread.id]
    })
    threads[thread.id] = thread
}
