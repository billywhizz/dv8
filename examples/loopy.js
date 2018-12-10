function dumpHandles () {
  const buf = Buffer.alloc(16384)
  const size = process.loop.listHandles(buf)
  const bytes = new Uint8Array(buf.bytes)
  let off = 0
  const handles = []
  while (1) {
    const active = bytes[off]
    const len = bytes[off + 1]
    if (len === 0) break
    const type = buf.read(off + 2, len)
    handles.push({ active, type })
    off += (2 + len)
  }
  print(JSON.stringify(handles, null, '  '))
  return size
}

global.onExit(() => dumpHandles())

const UV_RUN_ONCE = 1
const UV_RUN_NOWAIT = 2

const timer = setInterval(dumpHandles, 1000)
setTimeout(() => clearTimeout(timer), 5000)

while (1) {
  process.runMicroTasks()
  process.loop.run(UV_RUN_ONCE)
  if (!process.loop.isAlive()) {
    break
  }
}
