FROM python:3.7-buster

RUN apt-get update && apt-get install -y portaudio19-dev python-pyaudio python3-pyaudio
RUN pip install pvporcupine
RUN pip install pyalsaaudio
RUN git clone https://github.com/Picovoice/rhino /host_disk/src/rhino

COPY ./rhino.py /host_disk/src/rhino.py
COPY ./bees_linux_2020-02-18.rhn /host_disk/src/rhino.rhn
RUN cp /host_disk/src/rhino/lib/common/rhino_params.pv /host_disk/src/rhino.pv
RUN cp /host_disk/src/rhino/lib/linux/x86_64/libpv_rhino.so /host_disk/src/libpv_rhino.so

COPY ./main.py /host_disk/src/main.py
ENTRYPOINT python /host_disk/src/main.py