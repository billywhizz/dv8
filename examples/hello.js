const timer = setTimeout(() => {
    print(JSON.stringify(memoryUsage()))
    print(JSON.stringify(args))
    print(JSON.stringify(env))
}, parseInt(args[4] || '5') * 1000)