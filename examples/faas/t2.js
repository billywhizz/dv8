const timer = setInterval(() => {
    print('i am t2')
}, 1000)

setTimeout(() => clearTimeout(timer), 5000)