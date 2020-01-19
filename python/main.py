import pvporcupine
from rhino import Rhino
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

target_device_name = 'Jabra'
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


porcupine_handle = pvporcupine.create(
  keywords=['computer'],
  sensitivities=[0.9]
)
rhino_handle = Rhino(
  context_file_path="/host_disk/src/rhino.rhn",
  model_file_path="/host_disk/src/rhino.pv",
  library_path="/host_disk/src/libpv_rhino.so"
)
frame_len = porcupine_handle.frame_length

g = {}
g['woke'] = False
g['timer'] = time.time()
g['intent'] = False
def _audio_callback(in_data, frame_count, time_info, status):
  if frame_count >= frame_len:
    pcm = struct.unpack_from("h" * frame_len, in_data)
    should_wake = porcupine_handle.process(pcm)
    if should_wake and not g['intent']:
      g['woke'] = True
      g['timer'] = time.time()
      print('Listening')
    expired = time.time() - g['timer'] > 5
    if not should_wake and not g['intent'] and g['woke'] and expired:
      g['woke'] = False
      print('Stopped listening')
    if g['woke']:
      g['intent'] = rhino_handle.process(pcm)
    if g['intent']:
      if rhino_handle.is_understood():
        g['intent'], slot_values = rhino_handle.get_intent()
        print(g['intent'], slot_values)
      rhino_handle.reset()
      g['intent'] = False
      g['woke'] = False

  return None, pyaudio.paContinue

stream = pa.open(
channels=1,
format=pyaudio.paInt16,
rate=porcupine_handle.sample_rate,
frames_per_buffer=frame_len,
input=True,
input_device_index=target_device_desc['index'],
stream_callback=_audio_callback)

# print('Attached to sound input:', pa.get_default_input_device_info())
# stream.start_stream()

while True:
  time.sleep(float(0.1))
# time.sleep(1000.0)