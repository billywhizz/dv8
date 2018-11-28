module.exports = {
  platform: {
    bin: './out/bin',
    lib: './out/lib'
  },
  pods: [{
    name: 'pod1',
    cpu: '0.2',
    mem: '128m',
    threads: 20
  }, {
    name: 'pod2',
    cpu: '0.2',
    mem: '128m',
    threads: 20
  }, {
    name: 'pod3',
    cpu: '0.2',
    mem: '128m',
    threads: 20
  }, {
    name: 'pod4',
    cpu: '0.2',
    mem: '128m',
    threads: 20
  }, {
    name: 'pod5',
    cpu: '0.2',
    mem: '128m',
    threads: 20
  }],
  services: [{
    name: 'service1',
    mount: './examples/tls',
    path: './tlsHTTPServer2.js',
    args: [],
    env: { SERVICE_NAME: 'service1' }
  }, {
    name: 'service2',
    mount: './examples/tls',
    path: './tlsHTTPServer2.js',
    args: [],
    env: { SERVICE_NAME: 'service2' }
  }, {
    name: 'service3',
    mount: './examples/tls',
    path: './tlsHTTPServer2.js',
    args: [],
    env: { SERVICE_NAME: 'service3' }
  }]
}
