rm -f app/core
docker run -it --rm -v $(pwd)/app:/home/dv8 dv8-runtime dv8 $1