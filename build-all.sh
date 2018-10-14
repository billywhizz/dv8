docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/bin:/out/bin -v $(pwd)/src:/src dv8-sdk ./platform.sh
## build the standard modules
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh dns
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh event
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh fs
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh js
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh os
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh process
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh socket
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh thread
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh timer
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh tty
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh udp
docker run -it --rm -v $(pwd)/build:/build -v $(pwd)/out/lib:/out/lib -v $(pwd)/src:/src dv8-sdk ./module.sh vm