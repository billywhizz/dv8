## Goals
- download and build node.js
- optimize node.js build
- use generated libs from node.js build to comple out own js "platform"
- use docker to do the building and running
- compare sizes of binaries, stripped and unstripped
- compare size of RSS at startup
- comparse startup times (time to first JS line)
- use alpine base docker image

## Future Enhancements
- use buildroot

## Instructions
```
./build-docker.sh
./run.sh app.js
```

## Results
Binary Size = 17M
RSS with JS = 12.8MiB
