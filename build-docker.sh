#docker build -t v8-builder -f Dockerfile.v8 .
#docker run -d --rm v8-builder --name builder-copy /bin/true
#docker cp builder-copy:/tmp/v8 deps/
#docker build -t dv8-build -f ./Dockerfile.build .
docker run -it --rm -v $(pwd)/src:/source --workdir=/source dv8-build /bin/sh -c ./build.sh
docker build -t dv8-runtime -f ./Dockerfile.runtime .
