const { print, library } = dv8
const { OS } = library('os')
const os = new OS()
print(dv8.pid())
os.onSignal(signum => {
  print(`signal: ${signum}`)
  return 0
}, OS.SIGTERM)
print(JSON.stringify(dv8.listHandles()))
