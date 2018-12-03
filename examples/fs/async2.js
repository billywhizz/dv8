const { File, O_RDONLY } = module('fs', {})

function readFile (path, chunkSize = 4096, file = null) {
  if (!file) {
    const buf = Buffer.alloc(chunkSize)
    file = new File()
    file.setup(buf, buf)
    file.open(path, O_RDONLY)
    file.size = 0
    file.paused = true
    file.pause = () => (file.paused = true)
    file.resume = () => {
      file.paused = false
      process.nextTick(() => readFile(path, chunkSize, file))
    }
    return file
  }
  const len = file.read(chunkSize, file.size)
  if (len < 0) {
    file.close()
    file.onClose(new Error(`Read Error: ${len}`))
    return
  }
  if (len > 0) {
    if (file.onChunk) file.onChunk(len, file.size)
    file.size += len
    if (!file.paused) process.nextTick(() => readFile(path, chunkSize, file))
    return
  }
  file.close()
  file.onClose()
}

const path = process.args[2] || '/dev/zero'
const chunkSize = parseInt(process.args[3] || 4096, 10)
const file = readFile(path, chunkSize)
file.onClose = err => {
  if (err) {
    print(err.message)
    print(err.stack)
    file.close()
    return
  }
  const { PID, args, env, threads } = process
  const stats = process.stats()
  const mem = process.memoryUsage()
  print(JSON.stringify({
    PID, args, env, threads: threads.length, stats, mem, size: file.size
  }, null, '  '))
  clearTimeout(timer)
}
/*
file.onChunk = len => {
  //print(len)
  //print(file.size)
}
*/
file.resume()

let last = 0

const timer = setInterval(() => {
  print((file.size - last) / (1024 * 1024))
  last = file.size
}, 1000)
