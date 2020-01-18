#include <soundio/soundio.h>
#include "pv_porcupine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const float PI = 3.1415926535f;
static float seconds_offset = 0.0f;
static void read_callback(struct SoundIoInStream *instream,
        int frame_count_min, int frame_count_max)
{
    const struct SoundIoChannelLayout *layout = &instream->layout;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    int res;

    while (frames_left > 0) {
        int frame_count = frames_left;
        res = soundio_instream_begin_read(instream, &areas, &frame_count);
        if (res == SoundIoErrorIncompatibleDevice
          || res == SoundIoErrorStreaming 
          || res == SoundIoErrorInvalid) {
          fprintf(stderr, "%s\n", soundio_strerror(res));
          exit(1);
        }

        frames_left -= res;

        res = soundio_instream_end_read(instream);
        if (res == SoundIoErrorStreaming) {
            fprintf(stderr, "%s\n", soundio_strerror(res));
            exit(1);
        }

        if (frames_left == 0)
            break;
    }
}

int main(int argc, char **argv) {
    int err;
    struct SoundIo *soundio = soundio_create();
    if (!soundio) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    if ((err = soundio_connect(soundio))) {
        fprintf(stderr, "error connecting: %s", soundio_strerror(err));
        return 1;
    }

    soundio_flush_events(soundio);

    int default_input_device_index = soundio_default_input_device_index(soundio);
    if (default_input_device_index < 0) {
        fprintf(stderr, "no input device found");
        return 1;
    }

    struct SoundIoDevice *device = soundio_get_input_device(soundio, default_input_device_index);
    if (!device) {
        fprintf(stderr, "out of memory, could not get input device");
        return 1;
    }

    fprintf(stderr, "Input device: %s\n", device->name);

    struct SoundIoInStream *instream = soundio_instream_create(device);
    instream->format = SoundIoFormatS16LE;
    instream->sample_rate = 16000;
    instream->read_callback = read_callback;
    // instream->layout.channel_count = 1;
    err = soundio_instream_open(instream);

    if (err == SoundIoErrorInvalid) {
        fprintf(stderr, "SoundIoErrorInvalid");
        return 1;
    }
    if (err == SoundIoErrorOpeningDevice) {
        fprintf(stderr, "SoundIoErrorOpeningDevice");
        return 1;
    }
    if (err == SoundIoErrorNoMem) {
        fprintf(stderr, "SoundIoErrorNoMem");
        return 1;
    }
    if (err == SoundIoErrorBackendDisconnected) {
        fprintf(stderr, "SoundIoErrorBackendDisconnected");
        return 1;
    }
    if (err == SoundIoErrorSystemResources) {
        fprintf(stderr, "SoundIoErrorSystemResources");
        return 1;
    }
    if (err == SoundIoErrorNoSuchClient) {
        fprintf(stderr, "SoundIoErrorNoSuchClient");
        return 1;
    }
    if (err == SoundIoErrorIncompatibleBackend) {
        fprintf(stderr, "SoundIoErrorIncompatibleBackend");
        return 1;
    }
    if (err == SoundIoErrorIncompatibleDevice) {
        fprintf(stderr, "SoundIoErrorIncompatibleDevice");
        return 1;
    }

    if (instream->layout_error)
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(instream->layout_error));

    if ((err = soundio_instream_start(instream))) {
        fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
        return 1;
    }

    for (;;)
        soundio_wait_events(soundio);

    soundio_instream_destroy(instream);
    soundio_device_unref(device);
    soundio_destroy(soundio);
    return 0;
}