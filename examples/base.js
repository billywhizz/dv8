const { Process } = module('process', {})
const { Timer } = module('timer', {})

const process = new Process()
const mem = new Float64Array(4)

function getMemoryUsage() {
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
    t.start(fn, delay)
    return t
}

function setInterval(fn, repeat) {
    const t = new Timer()
    t.start(fn, repeat, repeat)
    return t
}

function clearTimeout(t) {
    t.stop()
}

global.setTimeout = setTimeout
global.setInterval = setInterval
global.clearTimeout = clearTimeout
global.clearInterval = clearTimeout
global.memoryUsage = getMemoryUsage

module.exports = {}
