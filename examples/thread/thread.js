const thread = process.spawn(() => {
  print('hello. i am a thread')
  process.nextTick(() => {
    print('closing on the next tick of the thread loop')
    process.sock.close()
  })
}, result => {
  print('thread says goodbye')
}, { ipc: true })
