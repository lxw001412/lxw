#!/bin/bash

systemctl stop star_pushersvr
systemctl disable star_pushersvr

cp star_pushersvr.service /usr/lib/systemd/system
mkdir -p /usr/local/comtom/star_pushersvr
cp star_pushersvr /usr/local/comtom/star_pushersvr
cp -rf media /usr/local/comtom/star_pushersvr
cp star_pushersvr.default.json /usr/local/comtom/star_pushersvr/star_pushersvr.json
mkdir -p /usr/local/comtom/starplatform_3rdlibs
cp -rf 3rd/* /usr/local/comtom/starplatform_3rdlibs/
cp -rf ffmpeg/* /usr/local/comtom/starplatform_3rdlibs/

systemctl enable star_pushersvr
systemctl start star_pushersvr
