require('./lib/base.js')
const start = Date.now()
const dv = new DataView(workerData)
const threadId = dv.getUint8(0)
const duration = dv.getUint8(1)
print(`thread: ${threadId} running for ${duration} seconds`)
const timer = setInterval(() => {
    dv.setBigUint64(2, BigInt(Date.now()))
    const elapsed = Math.floor((Date.now() - start) / 1000)
    if (elapsed >= duration) {
        clearTimeout(timer)
    }
}, 1000)
dv.setBigUint64(10, hrtime())
dv.setBigUint64(2, BigInt(Date.now()))
