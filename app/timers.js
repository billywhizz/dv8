const t1 = new Timer()
t1.start(() => {
    print('foo')
    t1.start(() => {
        print('baz')
    }, 2000)
}, 1000)
const t2 = new Timer()
t2.start(() => {
    print('bar')
}, 3000)
t2.stop()

const t3 = new Timer()
t3.start(() => {
    print(`Date: ${new Date()}`)
}, 1000, 1000)

const t4 = new Timer()
t4.start(() => {
    t3.stop()
}, 10000)