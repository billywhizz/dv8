
function printStats(pipe, direction = 'in') {
    const stats = new BigUint64Array(20)
    pipe.stats(stats)
    if(direction === 'in') {
        print(`close:     ${stats[0]}
error:     ${stats[1]}
read:      ${stats[2]}
pause:     ${stats[3]}
data:      ${stats[4]}
resume:    ${stats[5]}
end:       ${stats[6]}
`)
    } else {
        print(`close:      ${stats[0]}
error:      ${stats[1]}
written:    ${stats[10]}
incomplete: ${stats[11]}
full:       ${stats[12]}
drain:      ${stats[13]}
maxQueue:   ${stats[14]}
alloc:      ${stats[15]}
free:       ${stats[16]}
eagain:     ${stats[17]}
`)
    }
}

function createBuffer(size) {
    const buf = new Buffer()
    buf.bytes = buf.alloc(size) // buf.bytes is an instance of ArrayBuffer
    return buf
}

module.exports = { printStats, createBuffer }