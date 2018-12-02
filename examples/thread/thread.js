const thread = process.spawn(() => {
  print('hello')
}, result => {
  print('thread says goodbye')
})
