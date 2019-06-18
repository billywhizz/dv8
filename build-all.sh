./builtins.sh
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform-dynamic.sh $@
## build the standard modules
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh fs $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh libz $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh loop $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh os $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh process $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh socket $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh thread $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh timer $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh tty $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh udp $@
# build http parser
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./http-parser.sh httpParser $@
# build openssl
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./openssl.sh openssl $@
