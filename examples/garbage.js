const mem = new Float64Array(4)
const { Process } = module('process', {})
const { TTY } = module('tty', {})
const process = new Process()
const b = new Buffer()
b.alloc(100)
while(true) {
    process.memoryUsage(mem)
    gc()
}