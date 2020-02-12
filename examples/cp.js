const { print, require, args } = dv8
const { exec } = require('./spawn.js')

function spawn () {
  const p = exec({
    file: 'dv8',
    args: ['ping.js'],
    onExit: (status, pid) => {
      print(`${pid} exitCode ${status}`)
    }
  })
  p.stdout.onData(len => print(p.stdout.buffer.read(0, len), 0))
  print(p.pid)
  return p
}

const children = []
let todo = parseInt(args[2] || '1', 10)
while (todo--) {
  children.push(spawn())
}
