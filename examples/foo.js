require('./base.js')
const start = Date.now()
const data = new Uint8Array(workerData)
const [ id, duration ] = data
print(`thread: ${id} starting`)
const finish = start + (duration * 1000)
let now = start
while (now < finish) {
    const t = Math.floor(Date.now() / 1000)
    data[2] = (t >> 24) & 0xff;
    data[3] = (t >> 16) & 0xff;
    data[4] = (t >> 8) & 0xff;
    data[5] = t & 0xff;
    now = Date.now()
}
print(`thread: ${id} finished`)