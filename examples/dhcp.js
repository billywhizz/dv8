
const { UDP } = module('udp', {})

function createReceiver () {
  const sock = new UDP()
  const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
  sock.setup(rb, wb)
  sock.onClose(() => print('close'))
  let r = sock.bind('0.0.0.0', 68)
  print(`bind: ${r}`)
  sock.onMessage((len, address, port) => {
    print(`message from ${address}:${port}, ${len}`)
    //print(rb.read(0, len))
  })
  sock.start()
  return sock
}

function createMessage (buf) {
  // BOOTREQUEST
  buf[0] = 0x1
  // HTYPE ethernet
  buf[1] = 0x1
  // HLEN
  buf[2] = 0x6
  // HOPS
  buf[3] = 0x0
  // XID 4 bytes
  // buf[4] = (unsigned int) random()
  // SECS
  buf[8] = 0x0
  // FLAGS
  buf[10] = 0x80
  // CIADDR 12-15 is all zeros
  // YIADDR 16-19 is all zeros
  // SIADDR 20-23 is all zeros
  // GIADDR 24-27 is all zeros
  // CHADDR 28-43 is the MAC address, use your own
  buf[28] = 0xe4
  buf[29] = 0xce
  buf[30] = 0x8f
  buf[31] = 0x13
  buf[32] = 0xf6
  buf[33] = 0xd4
  // SNAME 64 bytes zero
  // FILE 128 bytes zero
  // OPTIONS
  // - magic cookie
  buf[236] = 99
  buf[237] = 130
  buf[238] = 83
  buf[239] = 99

  // DHCP Message type
  buf[240] = 53
  buf[241] = 1
  buf[242] = 1 // DHCPDISCOVER

  // DHCP Parameter request list
  buf[243] = 55
  buf[244] = 4
  buf[245] = 1
  buf[246] = 3
  buf[247] = 15
  buf[248] = 6
}

function createSender () {
  const sock = new UDP()
  const [rb, wb] = [Buffer.alloc(4096), Buffer.alloc(4096)]
  sock.setup(rb, wb)
  sock.onClose(() => print('close'))
  let r = sock.bind('0.0.0.0', 0)
  print(`bind: ${r}`)
  r = sock.setBroadcast(1)
  print(`broadcast: ${r}`)
  sock.onMessage((len, address, port) => {
    print(`message from ${address}:${port}, ${len}`)
    print(rb.read(0, len))
  })
  const bytes = new Uint8Array(wb.bytes)
  sock.start()
  process.nextTick(() => {
    createMessage(bytes)
    sock.send(256, '255.255.255.255', 67)
  })
  return sock
}

const receiver = createReceiver()
const sender = createSender()

