#!/bin/sh
ITER=${1:-1}
BLOCK_SIZE=${2:-1}
alias pipe="dv8 pipe.js"
alias count="dv8 count.js"
alias send="dv8 send.js"
alias recv="dv8 recv.js"
rm -f /tmp/pipe.sock && recv | count &
sleep 1
dd if=/dev/zero count=$ITER bs=$BLOCK_SIZE | send
unalias pipe
unalias count
unalias send
unalias recv