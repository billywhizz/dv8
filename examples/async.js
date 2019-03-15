process.nextTick(() => {
  throw new Error('From NextTick')
})

const timer = setTimeout(() => {
  clearTimeout(timer2)
  throw new Error('From Timeout')
}, 3000)

const timer2 = setInterval(() => {
  throw new Error('From Repeat Timer')
}, 1000)

const { Socket, TCP } = module('socket', {})
const client = new Socket(TCP)
const [ rb, wb ] = [ Buffer.alloc(1024), Buffer.alloc(1024) ]

function eventuallyThrowAsync (counter) {
  if (!counter--) throw new Error('From eventuallyThrowAsync')
  return process.nextTick(() => eventuallyThrowAsync(counter))
}

function eventuallyThrow (counter) {
  if (!counter--) throw new Error('From eventuallyThrow')
  //return process.nextTick(() => eventuallyThrow(counter))
  return eventuallyThrow(counter)
}

function iWillThrow () {
  setTimeout(() => {
    process.nextTick(() => {
      throw new Error('From nextTick in setTimeout in onConnect')
    })
    throw new Error('from setTimeout in onConnect')
  }, 1000)
}

client.onConnect(fd => {
  client.setup(rb, wb)
  print('client.connect')
  iWillThrow()
  eventuallyThrowAsync(8)
  eventuallyThrow(8)
  throw new Error('From onConnect')
})

client.onClose(() => {
  print('client.close')
})

client.connect('127.0.0.1', 53)
