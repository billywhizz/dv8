const cluster = require('cluster');

const WORKERS = parseInt(process.argv[2] || '1', 10)

if (cluster.isMaster) {
  console.log(`Master ${process.pid} is running`);
  const start = Date.now()
  for (let i = 0; i < WORKERS; i++) {
    cluster.fork();
  }
  cluster.on('exit', (worker, code, signal) => {
    console.log(`worker ${worker.process.pid} died`);
  });
  for (const id in cluster.workers) {
    cluster.workers[id].on('message', (m => {
        console.log(`boot ${m.pid}: ${m.boot - start} ms`)
    }));
  }
} else {
  console.log(`Worker ${process.pid} started`);
  process.send({ boot: Date.now(), pid: process.pid })
  setTimeout(() => {
      process.exit()
  }, 5000)
}