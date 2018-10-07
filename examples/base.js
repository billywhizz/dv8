/*
A PoC base module which exposes timers and memoryUsage
*/
const { Process } = module('process', {})
const { Timer } = module('timer', {})

const process = new Process()
const mem = new Float64Array(4)

function getMemoryUsage() {
    // read the values into the float array
    process.memoryUsage(mem)
    return {
        rss: mem[0],
        heapTotal: mem[1],
        heapUsed: mem[2],
        external: mem[3]
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

module.exports = {}
