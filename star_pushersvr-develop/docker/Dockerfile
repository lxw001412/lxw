FROM registry.comtom.cn:2443/publicimage/centos:7.4.1708-min
MAINTAINER pengyouwei<pengyouwei@comtom.cn>

ARG INSTALL_DIR=/usr/local/comtom/star_pushersvr
ARG LOG_DIR=/data/log/star_pushersvr

RUN mkdir -p ${INSTALL_DIR} \
    && mkdir -p ${LOG_DIR}

COPY star_pushersvr ${INSTALL_DIR}
ADD media ${INSTALL_DIR}/media
COPY star_pushersvr.default.json ${INSTALL_DIR}/star_pushersvr.json
ADD 3rd ${INSTALL_DIR}/3rd
ADD ffmpeg ${INSTALL_DIR}/ffmpeg

ENV LD_LIBRARY_PATH ${INSTALL_DIR}/3rd:${INSTALL_DIR}/ffmpeg

CMD ["/usr/local/comtom/star_pushersvr/star_pushersvr","-c","/usr/local/comtom/star_pushersvr/star_pushersvr.json"]

EXPOSE 6329/tcp