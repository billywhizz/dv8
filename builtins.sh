xxd -i src/base.js > src/builtins.h
sed -i 's/unsigned char/static const char/g' src/builtins.h
sed -i 's/unsigned int/static unsigned int/g' src/builtins.h
sed -i 's/examples_//g' src/builtins.h
xxd -i src/main.js > src/main.h
sed -i 's/unsigned char/static const char/g' src/main.h
sed -i 's/unsigned int/static unsigned int/g' src/main.h
sed -i 's/examples_//g' src/main.h
cat src/main.h >> src/builtins.h
rm src/main.h
