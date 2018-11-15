const { Socket, UNIX } = module('socket', {})
const { Parser } = require('./lib/ipc.js')
const { getMetrics } = require('./lib/metrics.js')

global.onUncaughtException = err => {
    print(err.message)
    print(err.stack)
}

const threads = {}

function spawn(fn, opts = { respawn: false }) {
    const { respawn } = opts
    const thread = createThread(fn, ({ err, thread, status }) => {
        delete threads[thread.id]
        print(`thread: ${thread.id}\ntime: ${thread.time}\nboot: ${thread.boot}\nstatus: ${status}`)
        if (err) {
            print(err.message)
            print(err.stack)
            return
        }
        if (respawn) nextTick(() => spawn(fn, opts))
    })
    threads[thread.id] = thread
    return thread
}

const listener = new Socket(UNIX)

listener.onConnect(fd => {
    const peer = new Socket(UNIX)
    const [ rb, wb ] = [ createBuffer(16384), createBuffer(16384) ]
    const parser = new Parser(rb, wb)
    peer.setup(fd, rb, wb)
    peer.onRead(len => {
        parser.read(len)
    })
    peer.onEnd(() => peer.close())
    parser.onMessage = message => {
        print(`main recv from ${message.threadId}`)
        const o = JSON.parse(message.payload)
        print(JSON.stringify(o))
        peer.write(parser.write({ shutdown: true }))
    }
})

listener.listen(`./${PID}.sock`)

spawn('./isolate.js', { respawn: true })
spawn('./isolate.js')
spawn('./isolate.js')
spawn('./isolate.js')

const timer = setInterval(() => {
    const running = Object.keys(threads).length
    print(`running: ${running}`)
    if (running === 0) {
        clearTimeout(timer)
        listener.close()
    }
}, 1000)