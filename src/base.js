/*
A PoC base module which exposes timers and memoryUsage
*/
const { Process } = module('process', {})
const { Timer } = module('timer', {})
const { Thread } = module('thread', {})

const process = new Process()
const mem = new Float64Array(16)
const cpu = new Float64Array(2)
const time = new BigInt64Array(1)
let next = 1
const thousand = BigInt(1000)
const million = BigInt(1000000)

const THREAD_BUFFER_SIZE = env.THREAD_BUFFER_SIZE || (4 * 1024)

const heap = [
    new Float64Array(4),
    new Float64Array(4),
    new Float64Array(4),
    new Float64Array(4),
    new Float64Array(4),
    new Float64Array(4),
    new Float64Array(4)
]

function cpuUsage() {
    process.cpuUsage(cpu)
    return {
        user: cpu[0],
        system: cpu[1]
    }
}

function hrtime() {
    process.hrtime(time)
    return time[0]
}

function heapUsage() {
    // read the values into the float array
    return process.heapUsage.call(process, heap)
}

function getMemoryUsage() {
    // read the values into the float array
    process.memoryUsage(mem)
    return {
        rss: mem[0],
        total_heap_size: mem[1],
        used_heap_size: mem[2],
        external_memory: mem[3],
        does_zap_garbage: mem[4],
        heap_size_limit: mem[5],
        malloced_memory: mem[6],
        number_of_detached_contexts: mem[7],
        number_of_native_contexts: mem[8],
        peak_malloced_memory: mem[9],
        total_available_size: mem[10],
        total_heap_size_executable: mem[11],
        total_physical_size: mem[12],
        isolate_external: mem[13]
    }
}

function setTimeout(fn, delay) {
    const t = new Timer()
    // the module will make the callback after delay
    t.start(fn, delay)
    // return the timer so it can be stopped. no gc/unref handling yet!
    return t
}

function setInterval(fn, repeat) {
    const t = new Timer()
    t.start(fn, repeat, repeat)
    return t
}

function clearTimeout(t) {
    // this will close the libuv handle for the timer
    t.stop()
}

function createBuffer(size) {
    const buf = new Buffer()
    buf.bytes = buf.alloc(size) // buf.bytes is an instance of ArrayBuffer
    return buf
}

function spawn(fun, onComplete) {
    const start = hrtime()
    const thread = new Thread()
    thread.buffer = createBuffer(THREAD_BUFFER_SIZE)
    const dv = new DataView(thread.buffer.bytes)
    thread.dv = dv
    thread.id = next++
    dv.setUint8(0, thread.id)
    const envJSON = JSON.stringify(env)
    const argsJSON = JSON.stringify(args)
    dv.setUint32(1, envJSON.length)
    thread.buffer.write(envJSON, 5)
    dv.setUint32(envJSON.length + 5, argsJSON.length)
    thread.buffer.write(argsJSON, envJSON.length + 9)
    thread.start(fun, (err, status) => {
        const finish = hrtime()
        const ready = dv.getBigUint64(0)
        thread.time = (finish - start) / thousand
        thread.boot = (ready - start) / thousand
        onComplete({ err, thread, status })
    }, thread.buffer)
    return thread
}

function dumpError(err) {
    print(
`Error
${err.message}
Stack
${err.stack}
`)
}

global.setTimeout = setTimeout
global.setInterval = setInterval
global.clearTimeout = clearTimeout
global.clearInterval = clearTimeout
global.memoryUsage = getMemoryUsage
global.heapUsage = heapUsage
global.cpuUsage = cpuUsage
global.hrtime = hrtime
global.createBuffer = createBuffer
global.createThread = spawn

global.onUncaughtException = err => {
    // log a trace?
    throw(err)
}

if (global.workerData) {
    global.workerData.bytes = global.workerData.alloc()
    const dv = new DataView(global.workerData.bytes)
    global.threadId = dv.getUint8(0)
    const envLength = dv.getUint32(1)
    const envJSON = global.workerData.read(5, envLength)
    global.env = JSON.parse(envJSON)
    const argsLength = dv.getUint32(5 + envLength)
    const argsJSON = global.workerData.read(9 + envLength, argsLength)
    global.args = JSON.parse(argsJSON)
    // set the boot time
    // TODO: why is this casting to bigint? isn't it returned as a bigint?
    dv.setBigUint64(0, hrtime())
} else {
    global.env = env().map(entry => entry.split('=')).reduce((env, pair) => { env[pair[0]] = pair[1]; return env }, {})
    global.threadId = 0
}

module.exports = {}
