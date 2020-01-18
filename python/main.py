import pvporcupine
from datetime import datetime
import pyaudio, time, struct

print('Loading...')
pa = pyaudio.PyAudio()
host_device_count = pa.get_device_count()
if host_device_count == 0:
  print('No host devices')
  exit(1)
else:
  print('Identified', host_device_count, 'devices')

target_device_name = 'C920'
target_device_desc = None
for idx in range(0, host_device_count):
  desc = pa.get_device_info_by_index(idx)
  print(idx, desc['name'])
  if target_device_name in desc['name']:
    print('Found target device', idx, desc['name'], desc)
    target_device_desc = desc
    break
if target_device_desc == None:
  print('Could not find target device', target_device_name)
  exit(1)


handle = pvporcupine.create(
  keywords=['computer'],
  sensitivities=[0.9]
)
frame_len = handle.frame_length

def _audio_callback(in_data, frame_count, time_info, status):
  if frame_count >= frame_len:
    pcm = struct.unpack_from("h" * frame_len, in_data)
    result = handle.process(pcm)
    if result:
      print('found', result)
  return None, pyaudio.paContinue

stream = pa.open(
channels=1,
format=pyaudio.paInt16,
rate=handle.sample_rate,
frames_per_buffer=frame_len,
input=True,
input_device_index=target_device_desc['index'],
stream_callback=_audio_callback)

# print('Attached to sound input:', pa.get_default_input_device_info())
# stream.start_stream()

while True:
  time.sleep(float(0.1))
# time.sleep(1000.0)