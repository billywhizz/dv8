# dv8 platform builder
docker build -t dv8-build -f ./Dockerfile.build .
docker run -it --rm -v $(pwd)/src:/src -v $(pwd)/build:/build -v $(pwd)/deps:/deps dv8-build ./platform.sh
docker build -t dv8 -f ./Dockerfile.run .
cp -f build/count app/count
