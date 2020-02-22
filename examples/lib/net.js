const { Socket } = dv8.library('net')

function socketPair (bufSize = 4096, fd) {
  const pipe = new Socket()
  const buf = Buffer.alloc(bufSize)
  if (fd) {
    pipe.fd = pipe.pair(fd)
  } else {
    pipe.fd = pipe.pair()
  }
  pipe.setup(buf, buf)
  pipe.buffer = buf
  return pipe
}

module.exports = { socketPair }
