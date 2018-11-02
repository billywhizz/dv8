const { Thread } = module('thread', {})

const t1 = new Thread()
t1.start('./t1.js', () => {
    print('t1 ended')
})

const t2 = new Thread()
t2.start('./t2.js', () => {
    print('t2 ended')
})
