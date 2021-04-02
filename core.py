import time
import numpy as np
import cv2
import os
import log
import utils

from cv_model import pred
from TrackedObject import TrackedObject
import databaseUpdate
import readUtils

def xywh2xyxy(xywh):
	return [xywh[0], xywh[1], xywh[0]+xywh[2], xywh[1]+xywh[3]]
def xyxy2xywh(xyxy):
	return [xyxy[0], xyxy[1], xyxy[2]-xyxy[0], xyxy[3]-xyxy[1]]
def xyxy2yxhw(xyxy):
	return [xyxy[1], xyxy[0], xyxy[3]-xyxy[1], xyxy[2]-xyxy[0]]

def processFrame(frame, trackingObjs, performPrediction, out, verbose=False):
	detection_threshold=0.4
	score_add_threshold = 0.6
	iou_threshold = 0.3
	deleteTrackedObjs = []
	IOU_vals = {}

	for trackerObj in trackingObjs:
		success, box = trackerObj.tracker.update(frame)
		if verbose:
			log.LOG_INFO("Box: ", box)
		if success:
			trackerObj.streakUntracked = 0
			trackerObj.updateBox(box)
			(x, y, w, h) = [int(v) for v in box]
			_ = cv2.rectangle(frame, (x, y), (x+w, y+h), trackerObj.color, 2)
			cv2.putText(frame, "ID: " + str(trackerObj.uuid)[:5], (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 1, trackerObj.color, 2)
		else:
			trackerObj.streakUntracked += 1
			if (trackerObj.streakUntracked > 0):
				deleteTrackedObjs.append(trackerObj)
			if verbose:
				log.LOG_ERR("Failed at frame", frame_no)

	out.write(frame)
	for obj in deleteTrackedObjs:
		trackingObjs.remove(obj)

	if performPrediction:
		output = pred.predict(frame)
		numDetections = np.where(np.array(output['scores']) > detection_threshold)[0].size
		output['scores'] = output['scores'][:numDetections]
		output['boxes'] = output['boxes'][:numDetections]
		output['classes'] = output['classes'][:numDetections]

		for index in range(len(output['boxes'])):
			currBox = output['boxes'][index]
			output['boxes'][index] = xyxy2yxhw([int(currBox[i]*frame.shape[i%2]) for i in range(len(currBox))])

		newTrackedObjs = []
		numAdditions = np.where(np.array(output['scores']) > score_add_threshold)[0].size
		if (numAdditions == 0):
			return
		
		for score_index in range(numAdditions):
			newTrackedObjs.append(score_index)

		#making a dictionary of IOU_vals
		#each (key, value) of the dictionary is (<uuid of tracked object to_curr>, <dictionary of IoUs>)
		#each (key, value) of <dictionary of IoUs> is (<index of bounding box bb_curr>, <IoU of to_curr.box and bb_curr>)
		
		log.LOG_INFO("New tracked objects: ", newTrackedObjs)
		log.LOG_INFO("Boxes: ", output['boxes'])
		for trackedObj in trackingObjs:
			IOU_vals[trackedObj.uuid] = {i:utils.computeIOU(list(trackedObj.bbox), output['boxes'][i]) for i in newTrackedObjs} if newTrackedObjs else None
		log.LOG_INFO(IOU_vals)

		#setting up list of newTrackedObjs to add (starts off full, and boxes that match existing TrackedObjects are gradually removed)
		#setting up list of deleteTrackedObjs (starts off empty, trackedObjects with no corresponding bounding box are added)
		#newTrackedObjs = list(range(<index of detections with score > score_add_threshold>) #TODO

		deleteTrackedObjs = []
		currTrackedObj = None

		for currTrackedObjUUID, trackedObjIoUs in IOU_vals.items():
			#currTrackedObj = <find tracked object with specified uuid>
			for trackedObj in trackingObjs:
				if trackedObj.uuid == currTrackedObjUUID:
					currTrackedObj = trackedObj
					
			#maxBoxIndex, maxIoU = <find max IoU in trackedObjIoUs, the box index that corresponds to it>
			maxIoU = trackedObjIoUs[list(trackedObjIoUs.keys())[0]] if len(trackedObjIoUs) != 0 else None
			if maxIoU != None:
				maxBoxIndex = list(IOU_vals[currTrackedObjUUID].keys())[0]
				for index in list(IOU_vals[currTrackedObjUUID].keys()):
					if IOU_vals[currTrackedObjUUID][index] > maxIoU:
						maxIoU = IOU_vals[currTrackedObjUUID][index]
						maxBoxIndex = index

				log.LOG_INFO("UUID: ",currTrackedObj.uuid)
				log.LOG_INFO("Bounding box: ", currTrackedObj.bbox)
				log.LOG_INFO("Maximum IOU: ", maxIoU)
				log.LOG_INFO("Max box index: ", maxBoxIndex)
				log.LOG_INFO("New tracked objects: ", newTrackedObjs)

				#We have a match between currTrackedObject and maxBox, and can update the box value
				if maxIoU > iou_threshold:
					maxBox = output['boxes'][maxBoxIndex]
					#convert maxBox to proper format
					currTrackedObj.updateBox(maxBox)

					log.LOG_INFO(IOU_vals)
					for uuid in list(IOU_vals.keys()):
						del IOU_vals[uuid][maxBoxIndex]
						
					newTrackedObjs.remove(maxBoxIndex)

				#No adequate bounding box is found for currTrackedObj, so it should be deleted
				else:
					deleteTrackedObjs.append(currTrackedObj)

		#removing every TrackedObject in deleteTrackedObjs
		#for trackedObjectToDelete in deleteTrackedObjs:
			#trackingObjs.remove(trackedObjectToDelete)

		#adding new TrackedObject for every bounding box index in newTrackedObj
		for newTrackedObj in newTrackedObjs:
			log.LOG_INFO("New Tracked Object: ", newTrackedObj)
			bbox = output['boxes'][newTrackedObj]
			trackerObj = TrackedObject(cv2.TrackerCSRT_create())
			trackerObj.updateBox(bbox)
			trackerObj.tracker.init(frame, tuple(bbox))
			trackingObjs.append(trackerObj)


	#Santript, this is commented because addToDatabase is not finished/tested
	#databaseUpdate.addToDatabase(trackingObjs)

def track():
	scaling = 6
	name = "video"
	fromLive = False
	writeOrig = False
	verbose = False
	if (fromLive):
		fps=10
		duration=15
		ip='10.0.0.148'

	frame_no = 0
	trackingObjs = []
	#creating the video capture for the input video
	if fromLive:
		cap = readUtils.readVideo(fps, duration, ip)
	else:
		cap = cv2.VideoCapture(name + "_orig.avi")

	#preparing the video out writer
	fourcc = cv2.VideoWriter_fourcc(*'XVID')
	height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
	width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
	fps = cap.get(cv2.CAP_PROP_FPS)
	if writeOrig:
		orig = cv2.VideoWriter(name + "_orig.avi", fourcc, fps, (width,height))
	out = cv2.VideoWriter(name + "_out.avi", fourcc, fps, (width,height))

	#Looping through every frame of the video
	while True:
		frame_no += 1
		ret, frame = cap.read()
		if not ret:
			break
		if writeOrig:
			orig.write(frame)
		processFrame(frame, trackingObjs, frame_no % scaling == 0, out, verbose=verbose)
		log.LOG_INFO("\nNext Frame", frame_no, "\n")

	cap.release()
	out.release()
	if writeOrig:
		orig.release()

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
