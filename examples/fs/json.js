const { readFile } = require('./fileSystem.js')

let chunks = []
const file = readFile('./foo.json')
const { buf } = file
file.onClose = err => {
  if (err) throw (err)
  const json = chunks.join('')
  const result = JSON.parse(json)
  print(JSON.stringify(result))
}
file.onChunk = len => {
  chunks.push(buf.read(0, len))
}
file.resume()
