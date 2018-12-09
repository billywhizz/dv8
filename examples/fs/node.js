const { readFile, readFileSync } = require('fs')
const { promisify } = require('util')
const { getPercentiles } = require('./stats.js')
const readFileAsync = promisify(readFile)
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms))
const nextTick = () => new Promise(resolve => process.nextTick(resolve))

const stats = []
let rate = 0

async function loadJSON (fname) {
  const buf = await readFileAsync(fname)
  const json = JSON.parse(buf)
  gc()
  return { size: buf.length, json }
}

function loadJSONSync (fname) {
  return new Promise((resolve, reject) => {
    const buf = readFileSync(fname, 'utf8')
    const json = JSON.parse(buf)
    setTimeout(() => resolve({ size: buf.length, json }), 0)
    gc()
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
    const start = process.hrtime.bigint()
    const { size, json } = await loadJSONSync(fileName)
    const elapsed = parseInt(((process.hrtime.bigint() - start) / 1000000n).toString())
    stats.push(elapsed)
    rate++
    if (stats.length > 1000) stats.shift()
    await nextTick()
  }
}

run(process.argv.slice(2)).catch(err => console.log(err.toString()))
