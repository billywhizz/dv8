# convert base js into c byte array so it can be built into binary
./builtins.sh $@
./build-runtime.sh $@
./build-module.sh httpParser $@
./build-module.sh picoHttpParser $@
