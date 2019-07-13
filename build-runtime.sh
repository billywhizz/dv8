# convert base js into c byte array so it can be built into binary
BUILDTYPE=${2:-static}
./builtins.sh
if [[ "$BUILDTYPE" == "static" ]]; then
  docker run -it --rm -v $(pwd)/.ccache:/root/.ccache -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh $@
else
  docker run -it --rm -v $(pwd)/.ccache:/root/.ccache -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform-dynamic.sh $@
fi