docker run -d --rm --name uv-build uv-build /bin/sleep 1000000
docker run -d --rm --name v8-build v8-build /bin/sleep 1000000
rm -fr deps/v8/src
rm -fr deps/v8/include
rm -fr deps/uv/src
rm -fr deps/uv/include
docker cp v8-build:/build/v8/out.gn/x64.release/obj/libv8_monolith.a deps/v8/
docker cp v8-build:/build/v8/include deps/v8/include
docker cp v8-build:/build/v8/src deps/v8/src
docker cp uv-build:/source/uv/include deps/uv/include
docker cp uv-build:/source/uv/out/Release/libuv.a deps/uv/libuv.a
docker cp uv-build:/source/uv/src deps/uv/src
docker stop uv-build
docker stop v8-build
