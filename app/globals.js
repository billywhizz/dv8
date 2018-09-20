require('./base.js')

setTimeout(() => {
    print("killing timer")
    t.stop()
}, 10000)

const t = setInterval(() => {
    print(new Date())
}, 1000)

