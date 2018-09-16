MODULE=${1:-os}
docker run -it --rm -v $(pwd)/src:/source/src --workdir=/source/src dv8-build /bin/sh -c ./module.sh $MODULE
cp -f src/$MODULE.so ./app/