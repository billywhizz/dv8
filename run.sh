rm -f app/core
docker run -it -p 3000:3000 --name dv8 --rm -v $(pwd)/app:/app dv8-runtime dv8 $1