MODULE=${1:-os}
docker run -it --rm -v $(pwd)/src:/source/src --workdir=/source/src -e MODULE_NAME=$MODULE dv8-build /bin/sh -c ./module.sh
cp -f src/$MODULE.so ./app/