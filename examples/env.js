const envVars = {}
const pairs = env()
for (const pair of pairs) {
    const [k, v] = pair.split('=')
    envVars[k] = v
}
print(JSON.stringify(envVars, null, '  '))
print(JSON.stringify(args, null, '  '))