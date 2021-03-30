import time
import numpy as np
import cv2
import os
import log

from cv_model import pred
from TrackedObject import TrackedObject
import databaseUpdate

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

def track(fps=10, duration=15, ip='10.0.0.148'):
	#cap = readVideo(fps, duration, ip)
	cap = cv2.VideoCapture("video_orig.avi")
	res, frame = cap.read()
	tracker = cv2.TrackerCSRT_create()
	output = pred.predict(frame)
	detection_threshold = 0.6
	trackingObjs = []
	fourcc = cv2.VideoWriter_fourcc(*'XVID')
	height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
	width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
	name = "video"
	#orig = cv2.VideoWriter(name + "_orig.avi", fourcc, fps, (width,height))
	out = cv2.VideoWriter(name + "_out.avi", fourcc, fps, (width,height))
	if len(output["scores"]) == 0:
		log.LOG_INFO("No Detections!!")

	for index in range(len(output["scores"])):
		score = output["scores"][index]
		bbox = output["boxes"][index]
		class_ = output["classes"][index]
		if score < detection_threshold:
			log.LOG_INFO("Exiting box search - No more objects")
			break
		bbox_temp = [0, 0, 0, 0]
		bbox_temp[0] = int(bbox[0]*frame.shape[0])
		bbox_temp[1] = int(bbox[1]*frame.shape[1])
		bbox_temp[2] = int(bbox[2]*frame.shape[0])
		bbox_temp[3] = int(bbox[3]*frame.shape[1])
		bbox = bbox_temp
		log.LOG_INFO(class_ + " is being tracked with probability " + str(score))
		log.LOG_INFO("Bounding box: " + str(tuple(bbox)))
		trackerObj = TrackedObject(cv2.TrackerCSRT_create())
		bbox = (bbox[1], bbox[0], bbox[3]-bbox[1], bbox[2]-bbox[0])
		trackerObj.tracker.init(frame, bbox)
		trackingObjs.append(trackerObj)
	frame_no = 1
	while True:
		ret, frame = cap.read()
		#orig.write(frame)
		deleteTrackedObjs = []
		if not ret:
			break
		for trackerObj in trackingObjs:
			success, box = trackerObj.tracker.update(frame)
			if success:
				(x, y, w, h) = [int(v) for v in box]
				_ = cv2.rectangle(frame, (x, y), (x+w, y+h), trackerObj.color, 2)
				cv2.putText(frame, "ID: " + str(trackerObj.uuid), (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 1, trackerObj.color, 2)
			else:
				deleteTrackedObjs.append(trackerObj)
				#print("Failed at frame", frame_no)
		frame_no += 1
		out.write(frame)
		for obj in deleteTrackedObjs:
			trackingObjs.remove(obj)
		#Santript, this is commented because addToDatabase is not finished/tested
        #databaseUpdate.addToDatabase(trackingObjs)
	cap.release()
	out.release()
	#orig.release()

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
