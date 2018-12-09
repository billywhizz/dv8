DV8_DNS="docker run -it --rm --name=dv8-dns --net=host -v $(pwd):/app billywhizz/dv8:0.0.5a dv8 ./dns.js"
## query google 8.8.8.8 dns server using UDP on port 53
$DV8_DNS www.google.com
## query local dnsmasq using UDP on port 53
$DV8_DNS www.google.com 127.0.0.53
## override port number in third param
$DV8_DNS www.google.com 127.0.0.53 53