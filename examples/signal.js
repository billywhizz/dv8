const { print, library } = dv8
const { OS } = library('os')
const os = new OS()
print(dv8.pid())
function signalHandler (signum) {
  print(`signal: ${signum}`)
  return 0
}
os.onSignal(signalHandler, 15)
//os.onSignal(signalHandler, 10)
