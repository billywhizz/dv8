const ANSI_RED = '\u001b[31m'
const ANSI_MAGENTA = '\u001b[35m'
const ANSI_DEFAULT = '\u001b[0m'
const ANSI_CYAN = '\u001b[36m'
const ANSI_GREEN = '\u001b[32m'

String.prototype.magenta = function (pad) { return `${ANSI_MAGENTA}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.red = function (pad) { return `${ANSI_RED}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.green = function (pad) { return `${ANSI_GREEN}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }
String.prototype.cyan = function (pad) { return `${ANSI_CYAN}${this.padEnd(pad, ' ')}${ANSI_DEFAULT}` }

const thousand = BigInt(1000)
const eight = BigInt(8)

function printStats(pipe, finish = Date.now()) {
  const { name, start } = pipe
  let mem = process.memoryUsage()
  const stats = new BigUint64Array(20)
  pipe.stats(stats)
  const [close, error, read, pause, data, resume, end, , , , written, incomplete, full, drain, maxQueue, alloc, free, eagain] = stats
  const elapsed = Math.max(finish - start, 1)
  const rMiBRate = read / BigInt(elapsed) / thousand
  const rMibRate = read * eight / BigInt(elapsed) / thousand
  const wMiBRate = written / BigInt(elapsed) / thousand
  const wMibRate = written * eight / BigInt(elapsed) / thousand
  print(`${name.red()}
${'time'.cyan(25)}${':'.cyan()} ${elapsed / 1000}
${'V8'.magenta()}
${'RSS'.green(25)}${':'.green()} ${mem.rss / (1024 * 1024)}
${'Heap'.green(25)}${':'.green()} ${mem.used_heap_size / (1024 * 1024)}
${'External'.green(25)}${':'.green()} ${mem.external_memory / (1024 * 1024)}
${'IO'.magenta()}
${'close'.cyan(25)}${':'.cyan()} ${close}
${'error'.cyan(25)}${':'.cyan()} ${error}
${'in'.magenta()}
${'Read Mib/sec'.green(25)}${':'.green()} ${rMibRate}
${'Read MiB/sec'.green(25)}${':'.green()} ${rMiBRate}
${'bytes'.cyan(25)}${':'.cyan()} ${read}
${'pause'.cyan(25)}${':'.cyan()} ${pause}
${'data'.cyan(25)}${':'.cyan()} ${data}
${'resume'.cyan(25)}${':'.cyan()} ${resume}
${'end'.cyan(25)}${':'.cyan()} ${end}
${'out'.magenta()}
${'Write Mib/sec'.green(25)}${':'.green()} ${wMibRate}
${'Write MiB/sec'.green(25)}${':'.green()} ${wMiBRate}
${'bytes'.cyan(25)}${':'.cyan()} ${written}
${'incomplete'.cyan(25)}${':'.cyan()} ${incomplete}
${'full'.cyan(25)}${':'.cyan()} ${full}
${'drain'.cyan(25)}${':'.cyan()} ${drain}
${'maxQueue'.cyan(25)}${':'.cyan()} ${maxQueue}
${'alloc'.cyan(25)}${':'.cyan()} ${alloc}
${'free'.cyan(25)}${':'.cyan()} ${free}
${'eagain'.cyan(25)}${':'.cyan()} ${eagain}
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
