print(JSON.stringify(args))
print(JSON.stringify(env))
setTimeout(() => {}, parseInt(args[4] || '2', 10) * 1000)