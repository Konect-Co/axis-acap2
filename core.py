import time
import numpy as np
import cv2
import os
import log
import utils

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
	iou_threshold = 0.5
	scaling = 6
	#score_add_threshold = 0.6
	trackingObjs = []
	fourcc = cv2.VideoWriter_fourcc(*'XVID')
	height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
	fps = cap.get(cv2.CAP_PROP_FPS)
	width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
	name = "video"
	outfps = int(1.0*fps/scaling)
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
		trackerObj.updateBox(bbox)
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
			#print("Box: ", box)
			trackerObj.updateBox(box)
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

		if frame_no % scaling != 0:
			continue

		output = pred.predict(frame)

		IOU_vals = {}

		for index in range(len(output['boxes'])):
			bbox_temp = [0, 0, 0, 0]
			bbox_temp[0] = int(output['boxes'][index][0]*frame.shape[0])
			bbox_temp[1] = int(output['boxes'][index][1]*frame.shape[1])
			bbox_temp[2] = int(output['boxes'][index][2]*frame.shape[0])
			bbox_temp[3] = int(output['boxes'][index][3]*frame.shape[1])
			output['boxes'][index] = [bbox_temp[1], bbox_temp[0], bbox_temp[3]-bbox_temp[1], bbox_temp[2]-bbox_temp[0]]

		newTrackedObjs = []
		for score_index in range(len(output['scores'])):
			if output['scores'][score_index] > detection_threshold:
				newTrackedObjs.append(score_index)

		#making a dictionary of IOU_vals
		#each (key, value) of the dictionary is (<uuid of tracked object to_curr>, <dictionary of IoUs>)
		#each (key, value) of <dictionary of IoUs> is (<index of bounding box bb_curr>, <IoU of to_curr.box and bb_curr>)
		print("New tracked objects: ", newTrackedObjs)
		for trackedObj in trackingObjs:
			print("Boxes: ")
			print(output['boxes'])
			IOU_vals[trackedObj.uuid] = {i:utils.computeIOU(list(trackedObj.bbox), output['boxes'][i]) for i in newTrackedObjs} if newTrackedObjs else None

		if(not newTrackedObjs):

			continue
		print(IOU_vals)

		#setting up list of newTrackedObjs to add (starts off full, and boxes that match existing TrackedObjects are gradually removed)
		#setting up list of trackedObjectsToDelete (starts off empty, trackedObjects with no corresponding bounding box are added)
		#newTrackedObjs = list(range(<index of detections with score > score_add_threshold>) #TODO

		trackedObjectsToDelete = []
		currTrackedObj = None

		for currTrackedObjUUID, trackedObjIoUs in IOU_vals.items():
			#currTrackedObj = <find tracked object with specified uuid> #TODO
			for trackedObj in trackingObjs:
				if trackedObj.uuid == currTrackedObjUUID:
					currTrackedObj = trackedObj
					
			#maxBoxIndex, maxIoU = <find max IoU in trackedObjIoUs, the box index that corresponds to it> #TODO
			maxIoU = trackedObjIoUs[list(trackedObjIoUs.keys())[0]] if len(trackedObjIoUs) != 0 else None
			if maxIoU != None:
				maxBoxIndex = list(IOU_vals[currTrackedObjUUID].keys())[0]
				for index in list(IOU_vals[currTrackedObjUUID].keys()):
					if IOU_vals[currTrackedObjUUID][index] > maxIoU:
						maxIoU = IOU_vals[currTrackedObjUUID][index]
						maxBoxIndex = index
				#print("maxBoxIndex: ", maxBoxIndex)
				#print()
				print("uuid: ",currTrackedObj.uuid)
				print("bounding box: ", currTrackedObj.bbox)
				print("Maximum IOU: ", maxIoU)
				print("Max box index: ", maxBoxIndex)
				print("new tracked objects: ", newTrackedObjs)
				#We have a match between currTrackedObject and maxBox, and can update the box value
				if maxIoU > iou_threshold:
					maxBox = output['boxes'][maxBoxIndex]
					#convert maxBox to proper format
					#currTrackedObj.bbox = maxBox
					currTrackedObj.updateBox(maxBox)

					#print("innerTrackedObjIoUs", innerTrackedObjIoUs)
					#print("UUID: ", currTrackedObjUUID)
					#print()

					print(IOU_vals)
					for uuid in list(IOU_vals.keys()):
						del IOU_vals[uuid][maxBoxIndex]

					#print(newTrackedObjs)
						
					newTrackedObjs.remove(maxBoxIndex)
				#No adequate bounding box is found for currTrackedObj, so it should be deleted
				else:
					print(currTrackedObj.uuid)
					trackedObjectsToDelete.append(currTrackedObj)

		#removing every TrackedObject in trackedObjectsToDelete
		for trackedObjectToDelete in trackedObjectsToDelete:
			trackingObjs.remove(trackedObjectToDelete)

		#adding new TrackedObject for every bounding box index in newTrackedObj
		print("Size of newTrackedObjs: ", len(newTrackedObjs))
		for newTrackedObj in newTrackedObjs:
			print("New Tracked Object: ", newTrackedObj)
			bbox = output['boxes'][newTrackedObj]
			trackerObj = TrackedObject(cv2.TrackerCSRT_create())
			trackerObj.updateBox(bbox)
			trackerObj.tracker.init(frame, tuple(bbox))
			trackingObjs.append(trackerObj)

		print()
		log.LOG_INFO("Next Frame", frame_no)
		print()

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
