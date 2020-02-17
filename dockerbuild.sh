#!/bin/bash

docker run --rm -it -v $PWD/..:/work -e DISPLAY=host.docker.internal:0 centos:test /bin/bash 

