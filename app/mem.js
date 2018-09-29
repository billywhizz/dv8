const memValues = new Float64Array(4)
function getMemoryUsage() {
    memoryUsage(memValues)
    return {
        rss: memValues[0],
        heapTotal: memValues[1],
        heapUsed: memValues[2],
        external: memValues[3]
    }
}
print(JSON.stringify(getMemoryUsage()))
gc()
print(JSON.stringify(getMemoryUsage()))
