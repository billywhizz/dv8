
//const dv = new DataView(workerData)
//print(`0: ${dv.getUint8()}`)
//print(JSON.stringify(args, null, '  '))
require('./base.js')
const shared = new Uint8Array(workerData)
const threadId = shared[0]
onExit(() => {
    print(`Exiting Thread: ${threadId}`)
})
const t = setTimeout(() => {

}, 1000)
