docker build -t dv8-build -f ./Dockerfile.build .
docker run -it --rm -v $(pwd)/src:/source/src --workdir=/source/src dv8-build /bin/sh -c ./build.sh
docker build -t dv8-runtime -f ./Dockerfile.runtime .