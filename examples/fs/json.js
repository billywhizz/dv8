const { Timer } = module('timer', {})
const { readFile } = require('./fileSystem.js')
const { getPercentiles } = require('./stats.js')
const timer = new Timer()
const sleep = ms => new Promise(resolve => timer.start(() => resolve(), ms))
const nextTick = () => new Promise(resolve => process.nextTick(resolve))

const stats = []
let rate = 0

function loadJSON (fname) {
  return new Promise((resolve, reject) => {
    let chunks = []
    const file = readFile(fname, { chunkSize: 65536 })
    const { buf } = file
    file.onClose = err => {
      if (err) return reject(err)
      const { size, fd, paused } = file
      resolve({ size, fd, paused, json: JSON.parse(chunks.join('')) })
      gc()
    }
    file.onChunk = len => chunks.push(buf.read(0, len))
    file.resume()
  })
}

async function run ([ fileName = './bbc.json' ]) {
  const timer = setInterval(() => {
    const percentiles = getPercentiles(stats, 1)
    const metrics = {
      top5: percentiles.slice(0, 5),
      bottom5: percentiles.slice(-5),
      rate
    }
    rate = 0
    console.log(JSON.stringify(metrics, null, '  '))
  }, 1000)
  while (1) {
    const start = process.hrtime()
    const { size, fd, paused, json } = await loadJSON(fileName)
    const elapsed = parseInt(((process.hrtime() - start) / 1000000n).toString())
    stats.push(elapsed)
    rate++
    if (stats.length > 1000) stats.shift()
    await nextTick()
  }
}

run(process.args.slice(2)).catch(err => console.log(err.toString()))
process.loop.ref()
