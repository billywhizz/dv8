function producerThread () {
  const MAX_SIZE = 1024
  const { Queue, PRODUCER } = module('posix', {})
  const producer = new Queue()
  const buf = Buffer.alloc(MAX_SIZE)
  producer.setup(buf)
  let status = producer.open('/ch6_ipc', PRODUCER)
  console.log(`producer.open: ${status}`)
  setInterval(() => {
    let len = producer.send(MAX_SIZE)
    console.log(`producer.send: ${len}`)
  }, 1000)
}

function consumerThread () {
  const MAX_SIZE = 1024
  const { Queue, CONSUMER } = module('posix', {})
  const consumer = new Queue()
  const buf = Buffer.alloc(MAX_SIZE)
  consumer.setup(buf)
  let status = consumer.open('/ch6_ipc', CONSUMER)
  console.log(`consumer.open: ${status}`)
  setInterval(() => {
    const len = consumer.recv(MAX_SIZE)
    console.log(`consumer.recv: ${len}`)
  }, 1000)
}

process.spawn(consumerThread, result => {})
process.spawn(producerThread, result => {})
