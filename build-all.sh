xxd -i src/base.js > src/builtins.h
sed -i 's/unsigned char/static const char/g' src/builtins.h
sed -i 's/unsigned int/static unsigned int/g' src/builtins.h
sed -i 's/examples_//g' src/builtins.h
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh $@
## build the standard modules
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh os $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh process $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh socket $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh thread $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh timer $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh tty $@
# build http parser
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./http-parser.sh httpParser $@
# build openssl
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./openssl.sh openssl $@