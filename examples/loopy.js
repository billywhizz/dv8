function dumpHandles () {
  print(JSON.stringify(process.activeHandles(), null, '  '))
}

process.onExit = dumpHandles

const UV_RUN_ONCE = 1

const timer = setInterval(dumpHandles, 1000)

setTimeout(() => clearTimeout(timer), 5000)

while (1) {
  process.runMicroTasks()
  process.loop.run(UV_RUN_ONCE)
  if (!process.loop.isAlive()) {
    break
  }
}
