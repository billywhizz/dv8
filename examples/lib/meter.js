require('./lib/base.js')

const ANSI_RED='\u001b[31m'
const ANSI_MAGENTA='\u001b[35m'
const ANSI_DEFAULT='\u001b[0m'
const ANSI_CYAN='\u001b[36m'
const ANSI_GREEN='\u001b[32m'

const million = 1024n * 1024n

function printStats(pipe, finish = Date.now()) {
    const { name, start } = pipe
    let mem = memoryUsage()
    const stats = new BigUint64Array(20)
    pipe.stats(stats)
    const [
        close,
        error,
        read,
        pause,
        data,
        resume,
        end,
        written,
        incomplete,
        full,
        drain,
        maxQueue,
        alloc,
        free,
        eagain
    ] = stats
    const elapsed = Math.max(finish - start, 1)
    const MiBRate = read / BigInt(elapsed) / 1000n
    const MibRate = read * 8n / BigInt(elapsed) / 1000n
    print(`${ANSI_MAGENTA}${name.padEnd(12, ' ')}
${ANSI_DEFAULT}${read} ${ANSI_CYAN}bytes
${ANSI_DEFAULT}${elapsed / 1000} ${ANSI_CYAN}sec${ANSI_DEFAULT}
${MibRate} ${ANSI_CYAN}Mib/s${ANSI_DEFAULT}
${MiBRate} ${ANSI_CYAN}MiB/s${ANSI_DEFAULT}
${mem.rss / (1024 * 1024)} ${ANSI_GREEN}RSS${ANSI_DEFAULT}
${mem.used_heap_size / (1024 * 1024)} ${ANSI_GREEN}Heap${ANSI_DEFAULT}
${mem.external_memory / (1024 * 1024)} ${ANSI_GREEN}Ext${ANSI_DEFAULT}`)
    print(`${ANSI_CYAN}close${ANSI_DEFAULT}      ${close}
${ANSI_CYAN}error${ANSI_DEFAULT}      ${error}
${ANSI_CYAN}read${ANSI_DEFAULT}       ${read}
${ANSI_CYAN}pause${ANSI_DEFAULT}      ${pause}
${ANSI_CYAN}data${ANSI_DEFAULT}       ${data}
${ANSI_CYAN}resume${ANSI_DEFAULT}     ${resume}
${ANSI_CYAN}end${ANSI_DEFAULT}        ${end}
${ANSI_CYAN}written${ANSI_DEFAULT}    ${written}
${ANSI_CYAN}incomplete${ANSI_DEFAULT} ${incomplete}
${ANSI_CYAN}full${ANSI_DEFAULT}       ${full}
${ANSI_CYAN}drain${ANSI_DEFAULT}      ${drain}
${ANSI_CYAN}maxQueue${ANSI_DEFAULT}   ${maxQueue}
${ANSI_CYAN}alloc${ANSI_DEFAULT}      ${alloc}
${ANSI_CYAN}free${ANSI_DEFAULT}       ${free}
${ANSI_CYAN}eagain${ANSI_DEFAULT}     ${eagain}
`)
}

module.exports = {
    start: (pipe, output) => {
        if (output) {
            pipe.timer = setInterval(() => printStats(pipe, Date.now(), 1000))
        }
        pipe.start = Date.now()
    },
    stop: (pipe) => {
        printStats(pipe, Date.now())
        if (pipe.timer) {
            pipe.timer.stop()
            delete pipe.timer
        }
    }
}
