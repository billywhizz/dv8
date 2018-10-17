#dd if=test.bin bs=65536 | dv8 pipe.js | count 
#cat test.bin | dv8 pipe.js | cat > copy.bin 
#diff test.bin copy.bin
#cat test.bin | dv8 pipe.js | cksum
#cat test.bin | cksum
dd if=/dev/urandom count=1638400 status=none bs=4096 > test.bin
cat test.bin | cksum
cat test.bin | dv8 pipe.js | cksum
cat test.bin | md5sum
cat test.bin | dv8 pipe.js | md5sum
rm -f test.bin