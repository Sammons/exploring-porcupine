FROM python:3.7-buster

RUN apt-get update && apt-get install -y portaudio19-dev python-pyaudio python3-pyaudio make cmake gcc
RUN git clone https://github.com/andrewrk/libsoundio /libsoundio && cd /libsoundio && mkdir build && cd build && cmake .. && make && make install

RUN pip install pvporcupine

COPY pv_porcupine.h picovoice.h /libsoundio/custom/
COPY main.c /libsoundio/custom/main.c

ENV PORCUPINE_LIB_FOLDER=/usr/local/lib/python3.7/site-packages/pvporcupine/lib/linux/x86_64/
ENV LD_LIBRARY_PATH=/libsoundio/build:${PORCUPINE_LIB_FOLDER}
RUN cd /libsoundio/custom/ && g++ -std=c++1y main.c -lsoundio -L${PORCUPINE_LIB_FOLDER} -l:libpv_porcupine.so && cp ./a.out ../build
# ENTRYPOINT /libsoundio/build/a.out