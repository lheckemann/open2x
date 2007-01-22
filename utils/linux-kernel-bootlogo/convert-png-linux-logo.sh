#!/bin/bash

./bin/pngtopnm-static ./logos/linux-logo-open2x-Khato.png \
        | ./bin/pnmtopng-static \
        | ./bin/png2linuxlogo-static \
        > ./output/linux_logo.h
