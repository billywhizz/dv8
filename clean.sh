rm -f build/dv8
rm -f build/*.so
rm -f app/*.so
rm -f build/*.o
rm -f build/*.a
rm -f build/count
rm -f app/count
sudo rm -f app/core
dockerclean
docker images | grep dv8