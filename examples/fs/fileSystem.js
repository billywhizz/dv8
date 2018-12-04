const { File, O_RDONLY, O_WRONLY, O_RDWR, O_APPEND, O_CREAT, O_TRUNC, O_EXCL } = module('fs', {})

function readFile (path, opts = {}, file = null) {
  const { chunkSize = 4096 } = opts
  if (!file) {
    const buf = Buffer.alloc(chunkSize)
    file = new File()
    file.setup(buf, buf)
    file.open(path, O_RDONLY)
    file.size = 0
    file.paused = true
    file.buf = buf
    file.pause = () => (file.paused = true)
    file.resume = () => {
      file.paused = false
      process.nextTick(() => readFile(path, opts, file))
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
    if (!file.paused) process.nextTick(() => readFile(path, opts, file))
    return
  }
  file.close()
  file.onClose()
}

module.exports = { readFile, File, O_RDONLY, O_WRONLY, O_RDWR, O_APPEND, O_CREAT, O_TRUNC, O_EXCL }
