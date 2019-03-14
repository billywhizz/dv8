ITER=1000000
BSIZE=65536
time dd if=/dev/zero count=$ITER bs=$BSIZE | dv8 count.js
time dd if=/dev/zero count=$ITER bs=$BSIZE | dv8 count2.js
time dd if=/dev/zero count=$ITER bs=$BSIZE | node ncount.js
time dd if=/dev/zero count=$ITER bs=$BSIZE | node ncount2.js
