const start = Date.now()
require('./base.js')
let mem = memoryUsage()
function toMiB(bytes) {
    return Math.floor((bytes / (1024 * 1024))*100) / 100
}
function toMib(bytes) {
    return Math.floor(((bytes * 8) / (1024 * 1024))*100) / 100
}
const ANSI_RED='\u001b[31m'
const ANSI_MAGENTA='\u001b[35m'
const ANSI_DEFAULT='\u001b[0m'
const ANSI_CYAN='\u001b[36m'
const ANSI_GREEN='\u001b[32m'
function printStats(pipe, end = Date.now()) {
    const { bytes, name } = pipe
    const elapsed = end - start
    const seconds = elapsed / 1000
    const MiBRate = Math.floor(toMiB(bytes) / seconds * 100) / 100
    const MibRate = Math.floor(toMib(bytes) / seconds * 100) / 100
    print(`${ANSI_MAGENTA}${name.padEnd(12, ' ')}${ANSI_DEFAULT}: ${bytes.toString().padStart(12, ' ')} ${ANSI_CYAN}bytes${ANSI_DEFAULT} ${seconds.toString().padStart(7, ' ')} ${ANSI_CYAN}sec${ANSI_DEFAULT} ${MibRate.toString().padStart(10, ' ')} ${ANSI_CYAN}Mib/s${ANSI_DEFAULT} ${MiBRate.toString().padStart(10, ' ')} ${ANSI_CYAN}MiB/s${ANSI_DEFAULT} ${toMiB(mem.rss.toString().padStart(10, ' '))} ${ANSI_GREEN}RSS${ANSI_DEFAULT} ${toMiB(mem.heapUsed.toString().padStart(10, ' '))} ${ANSI_GREEN}Heap${ANSI_DEFAULT} ${toMiB(mem.external.toString().padStart(10, ' '))} ${ANSI_GREEN}Ext${ANSI_DEFAULT}`)
}
const timer = setInterval(() => {
    mem = memoryUsage()
    //printStats(stdin, Date.now())
}, 1000)
const { TTY } = module('tty', {})
const b = new Buffer()
b.alloc(64 * 1024)
const stdin = new TTY(0, len => stdin.bytes += len, () => {
    printStats(stdin, Date.now())
    stdin.close()
    timer.stop()
}, () => {})
stdin.setup(b)
stdin.bytes = 0
stdin.name = 'count.stdin'
stdin.resume()
