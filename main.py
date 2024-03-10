import os
import struct

pipe_name = '/tmp/sensor_data'

with open(pipe_name, 'rb') as pipe_reader:
    while True:
        tag = int.from_bytes(pipe_reader.read(1), byteorder='little')

        if tag == 0:
            data = pipe_reader.read(2)
            x, y = struct.unpack('<BB', data)
            print(f"Point: ({x}, {y})")
        elif tag == 1:
            size = int.from_bytes(pipe_reader.read(1), byteorder='little')
            data = pipe_reader.read(size).decode('ascii')
            print(f"RandomString: size={size}, data='{data}'")
        else:
            print("Invalid data type tag encountered.")
            break
