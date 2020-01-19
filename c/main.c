#include "include/portaudio.h"
#include "include/pv_porcupine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        "/usr/local/lib/python3.7/site-packages/pvporcupine/lib/common/porcupine_params.pv",
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
        fprintf(stderr, "Reported error from porcupine: %s\n", error);
        // exit(2);
    }
}

#define SAMPLE_RATE 16000
#define NUM_CHANNELS 1
#define FRAME_PER_BUFFER 512 // identified by logging pv_porcupine_frame_length()

typedef struct {
  int frameIndex;
  int maxFrameIndex;
  int16_t * samples;
} data_frame;

static int record_callback(
  const void *inbuf,
  void *outbuf,
  uint64_t frames_per_buffer,
  const PaStreamCallbackTimeInfo* timing,
  PaStreamCallbackFlags status_flags,
  void * unused
) {
  const int max_frame_index = FRAME_PER_BUFFER * NUM_CHANNELS;
  const uint16_t* read_pointer = (const uint16_t*)inbuf;
  // assumes single channel PCM
  if (frames_per_buffer < FRAME_PER_BUFFER) {
    fprintf(stdout, "Nonstandard buffer size %d", frames_per_buffer);
  }
  memset(porcupine_input_buffer, 0, porcupine_input_buffer_size);
  memcpy(porcupine_input_buffer, read_pointer, frames_per_buffer);
  bool success = false;
  pv_status_t status = pv_porcupine_process(porcupine, porcupine_input_buffer, &success);
  if (success) {
    fprintf(stdout, "DETECTED\n");
  } else if (status) {
    fprintf(stderr, "PORCUPINE ERROR %s\n", pv_status_to_string(status));
    exit(1);
  }
  return paContinue;
}

void setup_pa() {
  PaError status = Pa_Initialize();
  if (status != paNoError) {
    fprintf(stderr, "Failed to initialize port audio");
    exit(1);
  }
  PaStreamParameters input_params, output_params;
  PaStream *stream = NULL;
  input_params.channelCount = NUM_CHANNELS;
  input_params.sampleFormat = paInt16;
  input_params.hostApiSpecificStreamInfo = NULL;
  PaDeviceIndex device_count = Pa_GetDeviceCount();
  fprintf(stdout, "Detected %d devices\n", device_count);
  PaDeviceIndex selected_device_idx = 0;
  const char* target_device = "Jabra";
  for (PaDeviceIndex idx = 0; idx < device_count; ++idx) {
    const PaDeviceInfo* info = Pa_GetDeviceInfo(idx);
    if (info != NULL) {
      fprintf(stdout, "Detected device at idx %d with name %s, channels\n", idx, info->name);
      if (strstr(info->name, target_device) != NULL) {
        fprintf(stdout, "Identified target device %s\n", target_device);
        selected_device_idx = idx;
        break;
      }
    }
  }
  input_params.device = selected_device_idx;
  status = Pa_OpenStream(&stream, &input_params, NULL /* output */, SAMPLE_RATE, FRAME_PER_BUFFER, paClipOff, record_callback, NULL);
  if (status != paNoError) {
    fprintf(stderr, "Failed to initialize stream: %s", Pa_GetErrorText(status));
    exit(1);
  }
  
  Pa_StartStream(stream);

  while (true) {
    status = Pa_IsStreamActive(stream);
    if (status < 0 && status != paNoError) {
      fprintf(stderr, "Failed to continue reading from stream: %d %s", status, Pa_GetErrorText(status));
    }
    Pa_Sleep(100);
  }
}

int main() {
  setup_porcupine();
  setup_pa();
}