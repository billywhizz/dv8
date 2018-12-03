const { readFile } = require('./fileSystem.js')

const path = process.args[2] || '/dev/zero'
const chunkSize = parseInt(process.args[3] || 4096, 10)
const file = readFile(path, { chunkSize })
let last = 0

file.onClose = err => {
  if (err) throw (err)
  print(`done, fileSize: ${file.size}`)
  clearTimeout(timer)
}

file.resume()

const timer = setInterval(() => {
  const rate = Math.floor((file.size - last) / (1024 * 1024))
  print(`rate: ${rate}, size: ${file.size}`)
  last = file.size
}, 1000)
