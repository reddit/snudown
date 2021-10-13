FROM python:3.8-buster

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install -y \
        python \
        python-dev \
        python-pip \
        build-essential \
        gperf \
        software-properties-common

ENV SRC_DIR /opt/snudown
ENV WHEEL_OUTPUT_DIR /tmp/dist

RUN mkdir -p $SRC_DIR $WHEEL_OUTPUT_DIR
WORKDIR $SRC_DIR

ADD . $SRC_DIR
CMD pip3 wheel --wheel-dir=$WHEEL_OUTPUT_DIR . ; pip2 wheel --wheel-dir=$WHEEL_OUTPUT_DIR .
