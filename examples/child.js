const { print, spawn, cwd, pid, library } = dv8
const { Socket } = library('net')
const { OS } = library('os')

function createPipe (name) {
  const pipe = new Socket()
  const buf = Buffer.alloc(65536)
  let bytes = 0
  pipe.onConnect(() => {
    print(`${name}.onConnect`)
  })
  pipe.onData(len => {
    bytes += len
    print(`${name}.onData: ${buf.read(0, len)}`, 0)
  })
  pipe.onEnd(() => {
    print(`${name}.onEnd bytes ${bytes}`)
  })
  pipe.fd = pipe.pair()
  pipe.setup(buf, buf)
  pipe.buffer = buf
  return pipe
}

function run () {
  const stdin = createPipe('stdin')
  const stdout = createPipe('stdout')
  const stderr = createPipe('stderr')
  stdin.onConnect(() => {
    setInterval(() => {
      stdin.write(stdin.buffer.write('ping'))
    }, 1000)
  })
  const child = spawn('bin/dv8', cwd(), ['examples/ping.js'], stdin.fd, stdout.fd, stderr.fd)
  print(`parent: ${pid()} child: ${child}`)
}

const os = new OS()

function signalHandler (signum) {
  print(`signal: ${signum}`)
  return 0
}

os.onSignal(signalHandler, 15)

run()
