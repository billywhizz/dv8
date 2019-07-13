if (!global.workerSource) {
  const ENV = global.env().map(entry => entry.split('=')).reduce((e, pair) => { e[pair[0]] = pair[1]; return e }, {})
  if (ENV.DV8_MODULES) {
    const _library = global.library
    global.library = (name, exports) => _library(name, exports, ENV.DV8_MODULES)
  }
}
const { EventLoop } = library('loop', {})
const loop = new EventLoop()
let source
if (global.workerSource) {
  const { workerSource } = global
  delete global.workerSource
  source = workerSource.slice(workerSource.indexOf('{') + 1, workerSource.lastIndexOf('}'))
} else {
  source = global.args[2]
}
function runLoop () {
  loop.run()
  do {
    loop.run()
  } while (loop.isAlive())
  loop.close()
}
Function('global', `"use strict";\n${source}`)(global) // eslint-disable-line no-new-func
runLoop()
