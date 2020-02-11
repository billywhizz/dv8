const { print, spawn, cwd, pid, library, args } = dv8
const { Socket } = library('net')

function createPipe (name) {
  const pipe = new Socket()
  const buf = Buffer.alloc(65536)
  let bytes = 0
  pipe.timer = setInterval(() => {
    if (pipe.paused) pipe.resume()
    pipe.paused = false
  }, 1000)
  pipe.onConnect(() => {
    print(`${name}.onConnect`)
  })
  pipe.onData(len => {
    bytes += len
    if (name === 'stderr') {
      print(buf.read(0, len))
    }
    pipe.pause()
    pipe.paused = true
  })
  pipe.onEnd(() => {
    print(`${name}.onEnd bytes ${bytes}`)
  })
  pipe.pair()
  pipe.setup(buf, buf)
  return pipe
}

function run () {
  const stdin = createPipe('stdin')
  const stdout = createPipe('stdout')
  const stderr = createPipe('stderr')
  //const child = spawn('/bin/ls', cwd(), ['-lah'], stdin, stdout, stderr)
  const child = spawn('/bin/dd', cwd(), ['if=/dev/zero', 'count=1000000', 'bs=4096'], stdin, stdout, stderr)
  //print(`parent: ${pid()} child: ${child}`)
}

run()
