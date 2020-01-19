#include <soundio/soundio.h>
#include "pv_porcupine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const float PI = 3.1415926535f;
static float seconds_offset = 0.0f;

static pv_porcupine_object_t* porcupine = NULL;
static int porcupine_frame_length = 0;
static int porcupine_input_buffer_size = 0;
static int16_t* porcupine_input_buffer = NULL;

static void setup_porcupine() {
    porcupine_frame_length = pv_porcupine_frame_length();
    porcupine_input_buffer_size = sizeof(int16_t) * porcupine_frame_length;
    porcupine_input_buffer = (int16_t*)malloc(porcupine_input_buffer_size);
    memset(porcupine_input_buffer, 0, porcupine_input_buffer_size);
    if (porcupine_input_buffer == NULL) {
        fprintf(stderr, "Failed to allocate buffer for porcupine input");
        exit(1);
    }
    pv_status_t status = pv_porcupine_init(
        NULL,
        "/usr/local/lib/python3.7/site-packages/pvporcupine/resources/keyword_files/linux/porcupine_linux.ppn",
        0.6,
        &porcupine
    );

    if (status != PV_STATUS_SUCCESS) {
        const char* error = 
            (status = PV_STATUS_IO_ERROR) ? "IO ERROR" :
            (status = PV_STATUS_INVALID_ARGUMENT) ? "INVALID ARGUMENT" :
            (status = PV_STATUS_OUT_OF_MEMORY) ? "OUT OF MEMORY" :
            "UNKNOWN";
        fprintf(stderr, "Reported error from porcupine: %s", error);
        // exit(2);
    }
}


// assumes setup_porcupine previously called
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
        int desired_num_samples = porcupine_input_buffer_size / instream->bytes_per_frame;
        static int porcupine_input_offset = 0;
        if (areas != NULL) {
            for (int f = 0; f < frame_count; ++f) {
                for (int ch = 0; ch < 1/* only want 1 channel */; ++ch) {
                    if (porcupine_input_offset*2 + instream->bytes_per_frame > porcupine_input_buffer_size) {
                        bool success = false;
                        int status = pv_porcupine_process(porcupine, porcupine_input_buffer, &success);
                        if (status) {
                            const char* msg = 
                                status == PV_STATUS_OUT_OF_MEMORY ? "OUT OF MEM" : 
                                status == PV_STATUS_INVALID_ARGUMENT ? "INVALID ARG" : "UNKNOWN";
                            fprintf(stderr, "PORCUPINE FAILED TO PROCESS %s\n", msg);
                        }
                        if (success) {
                            fprintf(stdout, "DETECTED");
                        }
                        porcupine_input_offset = 0;
                        memset(porcupine_input_buffer, 0, porcupine_input_buffer_size);
                    }
                    memcpy(
                         porcupine_input_buffer + porcupine_input_offset,
                         areas[ch].ptr,
                         instream->bytes_per_frame
                    );
                    porcupine_input_offset += (instream->bytes_per_frame / 2); // offset in 2byte sizes
                    areas[ch].ptr += areas[ch].step;
                }
            }
                    
        }

        frames_left -= frame_count;

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
    setup_porcupine();
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