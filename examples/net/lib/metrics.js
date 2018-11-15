function getMetrics() {
    return {
        threadId,
        env,
        args,
        mem: memoryUsage(),
        cpu: cpuUsage()
    }
}

module.exports = { getMetrics }