const timer = setInterval(() => {
    print('i am t1')
}, 1000)

setTimeout(() => clearTimeout(timer), 5000)