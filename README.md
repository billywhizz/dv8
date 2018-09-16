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
RSS without JS = 7.9MiB
RSS with JS = 14.7MiB

nodejs build is 26.3M on alpine docker
takes up 23M RSS when run