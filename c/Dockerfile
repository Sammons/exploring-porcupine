FROM python:3.7-buster

RUN apt-get update && apt-get install -y portaudio19-dev python-pyaudio python3-pyaudio make gcc libasound-dev
RUN pip install pvporcupine

RUN git clone https://git.assembla.com/portaudio.git /portaudio && cd /portaudio && ./configure && make && make install;


COPY include/ /workdir/include/
COPY main.c /workdir/main.c

ENV PORCUPINE_LIB_FOLDER=/usr/local/lib/python3.7/site-packages/pvporcupine/lib/linux/x86_64/
ENV PORTAUDIO_LIB_FOLDER=/portaudio/lib/.libs
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PORCUPINE_LIB_FOLDER}:${PORTAUDIO_LIB_FOLDER}
RUN cd workdir && g++ -std=c++1y main.c -lrt -lm -lasound -ljack -pthread -L${PORCUPINE_LIB_FOLDER} -l:libpv_porcupine.so -L${PORTAUDIO_LIB_FOLDER} -l:libportaudio.so

# ENTRYPOINT /libsoundio/build/a.out