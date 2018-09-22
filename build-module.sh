MODULE=${1:-os}
docker run -it --rm -v $(pwd)/src:/src -v $(pwd)/build:/build -v $(pwd)/deps:/deps dv8-build ./module.sh $MODULE
cp -f build/$MODULE.so ./app/