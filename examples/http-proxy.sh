export HTTPD_LISTEN_ADDRESS='127.0.0.1'
export HTTPD_LISTEN_PORT=3001
export PROXY_LOCAL_ADDRESS='0.0.0.0'
export PROXY_LOCAL_PORT=3000
export PROXY_REMOTE_ADDRESS='127.0.0.1'
export PROXY_REMOTE_PORT=3001
dv8 httpd.js &
dv8 spawn.js proxy2.js 2
#dv8 proxy2.js &