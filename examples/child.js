const { print, spawn, cwd, pid, library, args } = dv8
const { Socket } = library('net')

function createPipe (name) {
  const pipe = new Socket()
  const buf = Buffer.alloc(4096)
  pipe.onConnect(() => {
    print(`${name}.onConnect`)
  })
  pipe.onData(len => {
    print(`${name}.onData: ${len}`)
    print(buf.read(0, len))
  })
  pipe.onEnd(() => {
    print(`${name}.onEnd`)
  })
  pipe.fd = pipe.pair()
  pipe.setup(buf, buf)
  pipe.buf = buf
  return pipe
}

const stdin = createPipe('stdin', true)
const stdout = createPipe('stdout', true)
const stderr = createPipe('stderr', true)

const child = spawn('/bin/ls', cwd(), ['-lah'], stdin, stdout, stderr)
print(`parent: ${pid()} child: ${child}`)

//stdin.write(stdin.buf.write(args[2], 0))
