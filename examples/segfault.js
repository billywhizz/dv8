const b = new Buffer()
const ab = b.alloc(10)
b.write("hellodollywwioqoowwi", 0)
print(b.read(0, 100))
