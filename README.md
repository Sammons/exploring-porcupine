Porcupine https://github.com/Picovoice/porcupine

Is a semi-open source lib for wakeword analysis. It is pretty portable, and lightweight which is very nice. It is also pretty accurate just beating around with it myself on a mic. Note that you will need to update the main.py or other source file to replace the device name that is targeted for input. Also not all devices seem to work great with this code due to porcupine expecting a sample_rate of 16k and 1 input channel.

There are demos there but they are not as stripped down as I would have liked, and wanted to see if I could get this working myself.

### proof of concept running porcupine in python 3.7

* `cd python && docker build . --tag=porcpython`
* `docker run --device=/dev/snd -it porcpython sh`

If you do not want to run in Docker, then `pip install pvporcupine` before running. This was tested with python 3.7

### proof of concept running porcupine in node.js

I could not get this to work and opened an issue https://github.com/Picovoice/porcupine/issues/257