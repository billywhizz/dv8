./builtins.sh
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh $@

## build the standard modules
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh fs $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh loop $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh os $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh process $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh socket $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh thread $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh timer $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh tty $@
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh udp $@

## zlib module
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh libz $@

# pico http parser module
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./pico-http-parser.sh picoHttpParser $@

# http parser module
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./http-parser.sh httpParser $@

# openssl module
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./openssl.sh openssl $@
