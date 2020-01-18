FROM python:3.7-buster

RUN apt-get update && apt-get install -y portaudio19-dev python-pyaudio python3-pyaudio
RUN pip install pvporcupine
RUN pip install pyalsaaudio

COPY ./main.py /host_disk/src/main.py

ENTRYPOINT python /host_disk/src/main.py