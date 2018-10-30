const ab = new ArrayBuffer(10)
const bytes = new Uint8Array(ab)

if (console && console.log) {
    print = console.log
}

const request = {
    major: 0,
    minor: 0
}

({ 0:request.major, 1:request.minor } = bytes.slice(0, 2))

print(JSON.stringify(request))