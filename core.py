import time
import numpy as np
import re
import subprocess
import cv2
import os

from cv_model import pred


"""
Credit: https://stackoverflow.com/questions/7368739/numpy-and-16-bit-pgm/7369986#7369986
"""
def read_pgm(filename, byteorder='>'):
    """Return image data from a raw PGM file as numpy array.
    Format specification: http://netpbm.sourceforge.net/doc/pgm.html
    """
    with open(filename, 'rb') as f:
        buffer_ = f.read()
    try:
        header, width, height, maxval = re.search(
            b"(^P5\s(?:\s*#.*[\r\n])*"
            b"(\d+)\s(?:\s*#.*[\r\n])*"
            b"(\d+)\s(?:\s*#.*[\r\n])*"
            b"(\d+)\s(?:\s*#.*[\r\n]\s)*)", buffer_).groups()
    except AttributeError:
        raise ValueError("Not a raw PGM file: '%s'" % filename)
    return np.frombuffer(buffer_,
                         dtype='u1' if int(maxval) < 256 else byteorder+'u2',
                         count=int(width)*int(height),
                         offset=len(header)
                         ).reshape((int(height), int(width)))

def generateOutput():
	min_threshold = 0.80
	os.system("wget -q --user root --password pass123 \"http://10.0.0.148/mjpg/video.mjpg?fps=1&duration=2\" -O video.mjpeg")
	cap = cv2.VideoCapture("video.mjpeg")
	res, image = cap.read()
	image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
	if not res:
		print("Failed to read video.mjpeg")
		return
	#image = np.transpose(image, (1, 0, 2))
	output = pred.predict(image)
	detection = False


	for i in range(len(output["scores"])):
		score = output["scores"][i]
		if(score < min_threshold):
			break;
		detection = True
		print(output["classes"][i] + " detected with probability " +  str(score))

	#print(output)
	if not detection:
		print("No detection!!")
	print()

start_time = time.time()
seconds = 2;

measure1 = time.time()
measure2 = time.time()

while True:
	if measure2 - measure1 >= seconds:
		generateOutput()
		measure1 = measure2
		measure2 = time.time()
	else:
		measure2 = time.time()