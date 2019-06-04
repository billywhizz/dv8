const { Process } = library('process', {})
const time = new BigInt64Array(1)
const process = new Process()
process.hrtime(time)
const source = global.args[2]
Function('global', `"use strict";\n${source}`)(global) // eslint-disable-line no-new-func
const { EventLoop } = library('loop', {})
const loop = new EventLoop()
process.hrtime(time)
do {
  loop.run()
} while (loop.isAlive())
loop.close()
