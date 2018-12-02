const thread = process.spawn(() => {
  print('hello')
  //process.sock.close()
}, result => {
  print('thread says goodbye')
}, { ipc: false })
