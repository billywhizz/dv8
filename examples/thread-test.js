const { Thread } = module('thread', {})
const { OS } = module('os', {})

onExit(() => {
    print('exiting main')
})

function terminateHandler(signum) {
    print('main got SIGTERM')
    return 1
}

function spawn(fname) {
    return new Promise((ok, fail) => Thread().start(fname, e => e ? fail(e) : ok()))
}

const os = new OS()
const SIGTERM = 15
os.onSignal(terminateHandler, SIGTERM)
const [ , , fileName, workers ] = args

async function run() {
    const WORKERS = parseInt(workers || '4', 10)
    let i = WORKERS
    const threads = []
    while(i--) {
        threads.push(spawn(fileName))
    }
    await Promise.all(threads)
}

run().catch(err => print(`error: ${err.message}\nstack: ${err.stack}`))
