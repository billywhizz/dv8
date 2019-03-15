xxd -i src/base.js > src/builtins.h
sed -i 's/unsigned char/static const char/g' src/builtins.h
sed -i 's/unsigned int/static unsigned int/g' src/builtins.h
sed -i 's/examples_//g' src/builtins.h
