# convert base js into c byte array so it can be built into binary
BUILDTYPE=${1:-static}
xxd -i src/base.js > src/builtins.h
sed -i 's/unsigned char/static const char/g' src/builtins.h
sed -i 's/unsigned int/static unsigned int/g' src/builtins.h
sed -i 's/examples_//g' src/builtins.h
if [[ "$BUILDTYPE" == "static" ]]; then
  docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh $@
else
  docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform-dynamic.sh $@
fi