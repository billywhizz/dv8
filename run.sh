CONFIG=${1:-release}
if [[ "$CONFIG" == "release" ]]; then
    docker run -it --cap-add=NET_ADMIN --rm -v $(pwd)/out/bin:/usr/local/bin -v $(pwd)/out/lib:/usr/local/lib -v $(pwd)/examples:/app -p 3000:3000 dv8 /bin/sh
else
    docker run -it --cap-add=NET_ADMIN --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --rm -v $(pwd)/out/bin:/usr/local/bin -v $(pwd)/out/lib:/usr/local/lib -v $(pwd)/examples:/app -p 3000:3000 dv8-debug /bin/sh
fi
