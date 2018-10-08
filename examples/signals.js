const { OS } = module('os', {})

const os = new OS()

const SIGTERM = 15
const SIGHUP = 1
const SIGUSR1 = 30
const SIGINT = 2

function signalHandler(signum) {
    print(`i got SIGNAL: ${signum}`)
}

os.onSignal(signalHandler, SIGINT)
os.onSignal(signalHandler, SIGTERM)
os.onSignal(signalHandler, SIGHUP)
os.onSignal(signalHandler, SIGUSR1)
