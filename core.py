import time
import numpy as np
import cv2
import os
import log

from cv_model import pred

def readVideo(fps, duration, ip):
	log.LOG_INFO("Starting Read")
	wget_command = "wget -q --user root --password pass123 \"http://" + ip + "/mjpg/video.mjpg?fps=" + str(fps) + "&duration=" + str(duration) + "\" -O video.mjpeg"
	os.system(wget_command)
	cap = cv2.VideoCapture("video.mjpeg")
	log.LOG_INFO("Ending Read")
	return cap

def readImage(fps, duration, ip):
	cap = readVideo(fps, duration, ip)
	res, image = cap.read()
	image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
	if not res:
		log.LOG_ERR("Failed to read video.mjpeg")
		return
	return image

def track(fps, duration, ip):
	cap = readVideo(fps, duration, ip)
	res, frame = cap.read()
	tracker = cv2.TrackerCSRT_create()
	output = pred.predict(frame)
	index = 0
	bbox = output["boxes"][index]
	bbox_temp = [0, 0, 0, 0]
	bbox_temp[1] = int(bbox[0]*frame.shape[0])
	bbox_temp[0] = int(bbox[1]*frame.shape[1])
	bbox_temp[3] = int(bbox[2]*frame.shape[0])
	bbox_temp[2] = int(bbox[3]*frame.shape[1])
	bbox = bbox_temp
	log.LOG_INFO(output["classes"][index] + " is being tracked with probability " + str(output["scores"][index]))
	log.LOG_INFO("Bounding box: " + str(tuple(bbox)))
	tracker.init(frame, tuple(bbox))
	fourcc = cv2.VideoWriter_fourcc(*'XVID')
	height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
	width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
	name = "video"
	out = cv2.VideoWriter(name + "_out.avi", fourcc, fps, (width,height))
	frame_no = 1
	while True:
		ret, frame = cap.read()
		if not ret:
			break
		success, box = tracker.update(frame)
		if success:
			(x, y, w, h) = [int(v) for v in box]
			_ = cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
			out.write(frame)
		else:
			print("Failed at frame", frame_no)
			break
		frame_no += 1
	cap.release()
	out.release()


def generateOutput():
	min_threshold = 0.60

	image = readImage(10, 15, "10.0.0.148")
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

if __name__ == '__main__':
	track(10, 15, "10.0.0.148")

"""
if __name__ == "__main__":
	start_time = time.time()
	seconds = 3;

	measure1 = time.time()
	measure2 = time.time()

	while True:
		if measure2 - measure1 >= seconds:
			generateOutput()
			measure1 = measure2
			measure2 = time.time()
		else:
			measure2 = time.time()
"""