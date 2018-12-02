const { UDP } = module('udp', {})
const sock = new UDP()
const [ rb, wb ] = [ Buffer.alloc(4096), Buffer.alloc(4096) ]
sock.setup(rb, wb)
sock.onClose(() => {
  print('close')
})
sock.send(wb.write('0123456789', 0), '0.0.0.0', 30000)
sock.close()
