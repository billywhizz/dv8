const b = new Buffer()

b.ab = new Uint8Array(b.alloc(100))

print(b.ab.length)
print(b.ab[0])
