const { TTY } = module('tty', {})
const start = Date.now()
const BUFFER_SIZE = 64 * 1024
const b = new Buffer()
b.alloc(BUFFER_SIZE)

function toMib(bytes) {
    return bytes * 8n / (1024n * 1024n)
}

function stdinStats() {
    const st = new BigUint64Array(7)
    stdin.stats(st)
    return {
        read: st[0],
        error: st[1],
        pause: st[2],
        close: st[3],
        data: st[4],
        resume: st[5],
        end: st[6],
    }
}

function dumpStdin() {
    const stats = stdinStats()
print(`
-------------------------------
stdin
-------------------------------
read:        ${stats.read}
error:       ${stats.error}
pause:       ${stats.pause}
close:       ${stats.close}
data:        ${stats.data}
resume:      ${stats.resume}
end:         ${stats.end}
-------------------------------
`)
}

function stdoutStats() {
    const st = new BigUint64Array(10)
    stdout.stats(st)
    return {
        written: st[0],
        incomplete: st[1],
        full: st[2],
        error: st[3],
        drain: st[4],
        close: st[5],
        maxQueue: st[6],
        alloc: st[7],
        free: st[8],
        eagain: st[9]
    }
}

function dumpStdout() {
    const stats = stdoutStats()
print(`
-------------------------------
stdout
-------------------------------
written:     ${stats.written}
incomplete:  ${stats.incomplete}
full:        ${stats.full}
error:       ${stats.error}
drain:       ${stats.drain}
close:       ${stats.close}
maxQueue:    ${stats.maxQueue}
alloc:       ${stats.alloc}
free:        ${stats.free}
eagain:      ${stats.eagain}
-------------------------------
`)
}

function dumpMetrics(metrics) {
    print(`${new Date()} r: ${toMib(metrics.readRate)} Mib w: ${toMib(metrics.writeRate)} Mib`)
}

function getMetrics() {
    const finish = Date.now()
    const elapsed = finish - start
    const elapsedSeconds = BigInt(Math.ceil(elapsed / 1000))
    const stdin = stdinStats()
    const stdout = stdoutStats()
    const readRate = stdin.read / elapsedSeconds
    const writeRate = stdout.written / elapsedSeconds
    return { start, finish, stdin, stdout, elapsed, elapsedSeconds, readRate, writeRate }
}

function verify() {
    const { start, finish, stdin, stdout, elapsed, elapsedSeconds, readRate, writeRate } = getMetrics()
    print(`no errors      : ${stdin.error === 0n && stdout.error === 0n}`)
    print(`read = written : ${stdin.read === stdout.written}`)
    print(`allocations    : ${stdout.alloc === stdout.free}`)
    print(`allocations 2  : ${stdout.alloc === (stdout.eagain + stdout.incomplete)}`)
    print(`writes = reads : ${stdin.data === (stdout.full + stdout.incomplete + stdout.eagain)}`)
    print(`stdin.end      : ${stdin.end === 1n}`)
    print(`stdin.close    : ${stdin.close === 1n}`)
    print(`stdout.close   : ${stdout.close === 1n}`)
    print(`pause = resume : ${stdin.pause === stdin.resume}`)
}

const stdin = new TTY(0, len => {
    const r = stdout.write(len)
    if (r < 0) return stdout.close()
    if (r < len && stdout.queueSize() >= (64 * 1024)) stdin.pause()
}, () => stdin.close(), () => stdout.close())
const stdout = new TTY(1, () => {
    timer.stop()
    const metrics = getMetrics()
    dumpMetrics(metrics)
    verify()
}, () => stdin.resume(), e => { print(`write error: ${e}`) })
stdin.setup(b)
stdout.setup(b)
stdin.resume()
const timer = new Timer()
timer.start(() => dumpMetrics(getMetrics()), 1000, 1000)
