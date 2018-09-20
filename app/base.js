const { Process } = module('process', {})

const process = new Process()

function getMemoryUsage() {
    const memValues = new Float64Array(4)
    process.memoryUsage(memValues)
    return {
        rss: memValues[0],
        heapTotal: memValues[1],
        heapUsed: memValues[2],
        external: memValues[3]
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