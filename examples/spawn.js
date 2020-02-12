const { spawn, cwd, waitpid, constants, require } = dv8
const { EINTR } = constants
const { socketPair } = require('./net.js')

const answer = new Int32Array(new ArrayBuffer(8))

function exec (opts) {
  const { file, args, workDir = cwd(), bufSize, onExit = () => {} } = opts
  const stdin = socketPair(bufSize)
  const stdout = socketPair(bufSize)
  const stderr = socketPair(bufSize)
  const child = spawn(file, workDir, args, stdin.fd, stdout.fd, stderr.fd)
  function checkPID () {
    const [status, rc] = waitpid(answer, child)
    if (rc === 0) return setTimeout(checkPID, 100)
    if (rc === -1 && (errno === EINTR)) return setTimeout(checkPID, 100)
    onExit(status, rc)
  }
  stderr.onEnd(checkPID)
  return { pid: child, stdin, stdout, stderr, file, args, workDir }
}

module.exports = { exec }
