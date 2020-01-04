# convert base js into c byte array so it can be built into binary
./builtins.sh $@
docker run -it --rm -v $(pwd)/.ccache:/root/.ccache -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh $@
