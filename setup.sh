## v8 builder
docker build -t v8-build -f Dockerfile.v8 .
docker run -d --name v8-build --rm v8-build /bin/sleep 3600
mkdir -p deps/v8/out.gn
docker cp v8-build:/tmp/v8/include deps/v8/include
docker cp v8-build:/tmp/v8/out.gn/x64.release deps/v8/out.gn/x64.release
docker kill v8-build
## uv builder
docker build -t uv-build -f Dockerfile.uv .
docker run -d --name uv-build --rm uv-build /bin/sleep 3600
mkdir -p deps/uv/out
docker cp uv-build:/source/uv/include deps/uv/include
docker cp uv-build:/source/uv/out/Release deps/uv/out/Release
docker kill uv-build
# dv8 platform builder
docker build -t dv8-build -f ./Dockerfile.build .
