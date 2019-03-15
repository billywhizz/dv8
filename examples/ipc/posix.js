const { OS } = module('os', {})

const SIGPIPE = 13
const os = new OS()
os.onSignal(signum => {
  return 0
}, SIGPIPE)

function producerThread () {
  const MAX_SIZE = 64 * 1024
  const { Queue, PRODUCER } = module('posix', {})
  const producer = new Queue()
  const buf = Buffer.alloc(MAX_SIZE)
  producer.setup(buf)
  let status = producer.open('/foobarbaz', PRODUCER)
  console.log(`producer.open: ${status}`)
  let total = 0
  function start () {
    const len = producer.send(MAX_SIZE)
    if (len > 0) {
      //console.log(`producer.send: ${len}`)
      total += len
    }
    setTimeout(start, 1)
  }
  const timer = setInterval(() => {
    total = Math.floor((total / (1024 * 1024)) * 100) / 100
    console.log(`producer.total: ${total}`)
    total = 0
  }, 1000)
  start()
}

function consumerThread () {
  const MAX_SIZE = 64 * 1024
  const { Queue, CONSUMER } = module('posix', {})
  const consumer = new Queue()
  const buf = Buffer.alloc(MAX_SIZE)
  consumer.setup(buf)
  let status = consumer.open('/foobarbaz', CONSUMER)
  console.log(`consumer.open: ${status}`)
  process.send({ })
  let total = 0
  function poll () {
    const len = consumer.recv(MAX_SIZE)
    if (len > 0) {
      //console.log(`consumer.recv: ${len}`)
      total += len
    }
    process.nextTick(poll)
  }
  const timer = setInterval(() => {
    total = Math.floor((total / (1024 * 1024)) * 100) / 100
    console.log(`consumer.total: ${total}`)
    total = 0
  }, 1000)
  poll()
}

const consumer = process.spawn(consumerThread, result => {}, { ipc: true })
consumer.onMessage(message => {
  process.spawn(producerThread, result => {}, { ipc: true })
})
