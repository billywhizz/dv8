/*
A PoC base module which exposes timers and memoryUsage
*/
const { Process } = module('process', {})
const { Timer } = module('timer', {})

const process = new Process()
const mem = new Float64Array(16)
const cpu = new Float64Array(2)
const time = new BigInt64Array(1)

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

global.setTimeout = setTimeout
global.setInterval = setInterval
global.clearTimeout = clearTimeout
global.clearInterval = clearTimeout
global.memoryUsage = getMemoryUsage
global.heapUsage = heapUsage
global.cpuUsage = cpuUsage
global.hrtime = hrtime

global.threadId = 0

if (global.workerData) {
    const bytes = new Uint8Array(global.workerData)
    global.threadId = bytes[0]
}
module.exports = {}
