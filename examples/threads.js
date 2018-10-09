require('./base.js')
const { Thread } = module('thread', {})

const thread = new Thread()
const b = new Buffer()
const ab = new Uint8Array(b.alloc(10))
thread.createContext(b, () => {
    print('done')
    timer.stop()
})
const timer = setInterval(() => {
    print(JSON.stringify(ab))
    gc()
}, 1000)