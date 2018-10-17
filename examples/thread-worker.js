require('./lib/base.js')
const start = Date.now()
const dv = new DataView(workerData)
const id = dv.getUint8(0)
const duration = dv.getUint8(1)
print(`thread: ${id} running for ${duration} seconds`)
const timer = setInterval(() => {
    const t = Date.now()
    dv.setFloat64(2, t)
    dv.setFloat64(10, ready)
    const elapsed = Math.floor((t - start) / 1000)
    if (elapsed >= duration) {
        clearTimeout(timer)
    }
}, 1000)
const ready = Date.now()