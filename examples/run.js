const { print, require, args } = dv8
const { exec } = require('./lib/spawn.js')

const p = exec({
  file: args[2],
  args: args.slice(3),
  onExit: (status, pid) => {
    print(`${pid} exitCode ${status}`)
  }
})
p.stdout.onData(len => print(p.stdout.buffer.read(0, len), 0))
