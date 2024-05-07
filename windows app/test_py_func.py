import pyaudio
defaultFrames = 16
paObj = pyaudio.PyAudio()
try:
    defaultOutputDeviceHandle = paObj.get_default_output_device_info()
    # defaultOutputDeviceHandle = paObj.get_device_info_by_index(idx)
except IOError:
    print ("No default device available. Quitting...")
    exit(-1)

stream = paObj.open(format = pyaudio.paInt16,
                    channels = defaultOutputDeviceHandle["maxOutputChannels"],
                    rate = int(defaultOutputDeviceHandle["defaultSampleRate"]),
                    input = True,
                    frames_per_buffer = defaultFrames,
                    input_device_index = defaultOutputDeviceHandle["index"],
                    as_loopback = True)

recordedFrames = list(stream.read(defaultFrames))
stream.stop_stream()
stream.close()
paObj.terminate()

joinedFrames = []
for i in range(0, len(recordedFrames), 2):
    joinedFrames.append(recordedFrames[i] << 8 | recordedFrames[i+1])


print((max(joinedFrames) - min(joinedFrames)) * 100 / (1 << 16))

