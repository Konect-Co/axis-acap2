import time
import numpy as np
import cv2
import os
import log
import utils
from cv_model import pred

from mtcnn.mtcnn import MTCNN
from TrackedObject import TrackedObject
import databaseUpdate
import readUtils

face_detector = MTCNN()
demographics_model = load_model('KonectDemographics.h5')

def xywh2xyxy(xywh):
	return [xywh[0], xywh[1], xywh[0]+xywh[2], xywh[1]+xywh[3]]
def xyxy2xywh(xyxy):
	return [xyxy[0], xyxy[1], xyxy[2]-xyxy[0], xyxy[3]-xyxy[1]]
def xyxy2yxhw(xyxy):
	return [xyxy[1], xyxy[0], xyxy[3]-xyxy[1], xyxy[2]-xyxy[0]]

def interpret_demographics_label(age_label, gender_label, race_label):
	races = ["White", "Black", "Asian", "Indian", "Other"]
	age = int(age_label[0]*116)
	gender = "male" if gender_label[0]<0.5 else "female"
	#print(races, race_label.flatten())
	#print(gender_label[0])
	race = races[np.argmax(race_label.flatten())]

	return age, gender, race

def processFrame(frame, trackingObjs, performPrediction, out, verbose=False):
	detection_threshold=0.3
	score_add_threshold = 0.3
	iou_threshold = 0.3
	face_ioa_threshold = 0.95
	deleteTrackedObjs = []
	IOU_vals = {}

	if performPrediction:
		output = pred.predict(frame)

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
		for trackedObjectToDelete in deleteTrackedObjs:
			databaseUpdate.deleteTrackingObject(trackedObjectToDelete)
			trackingObjs.remove(trackedObjectToDelete)

		#adding new TrackedObject for every bounding box index in newTrackedObj
		for newTrackedObj in newTrackedObjs:
			log.LOG_INFO("New Tracked Object: ", newTrackedObj)
			bbox = output['boxes'][newTrackedObj]
			trackerObj = TrackedObject(cv2.TrackerCSRT_create())
			trackerObj.updateBox(bbox)
			trackerObj.tracker.init(frame, tuple(bbox))
			trackingObjs.append(trackerObj)

	#TODO: Santript, I wrote the demographics updating code here, but commented it since untested
	#Maybe we can go through it together and make it work
	"""
	#making a copy so that when TrackedObject is removed from this list, it remains in original
	trackingObjsDemographics = copy(trackingObjs)
	#obtaining face detections for current frame
	faces = detector.detect_faces(frame)
	for face in faces:
		#index to iterate through the tracking objects
		i = 0
		while i < len(trackingObjsDemographics):
			#obtaining the current tracking object
			trackingObj = trackingObjsDemographics[i]

			#obtaining both boxes in x, y, w, h format
			face_box = face['box']
			face_x, face_y, face_w, face_h = face_box
			tracking_box = trackingObj.bbox

			#computing IoA for the face, to determine whether there is a match
			ioa = utils.computeIOA(face_box, tracking_box)
			if (ioa > face_ioa_threshold):
				#obtaining the ROI for the face
				face_roi = frame[face_x, face_y, face_x+face_w, face_y+face_h]
				face_roi = cv2.resize(frame_roi, (150, 150))
				
				#predicting demographics
				face_demographics = demographics_model.predict(face_roi)
				age, gender, race = interpret_demographics_label(face_demographics[0][0], face_demographics[1][0], face_demographics[2][0])
				
				#Updating the demographics of the tracking object=
				trackingObject.age.append(age)
				trackingObject.gender.append(gender)
				trackingObject.race.append(race)

				#Removing the object so it is not updated again in the current frame
				trackingObjsDemographics.remove(trackingObj)
				continue

			#Increasing i if there is no matching face found
			i++
	"""

	#TODO: We now need to update the database based on the demographics predictions that are coming in
	#Santript, this is commented because addToDatabase is not finished/tested
	databaseUpdate.addToDatabase(trackingObjs)

def track():
	scaling = 6
	name = "video"
	fromLive = False
	writeOrig = False
	verbose = False
	if (fromLive):
		fps = 10
		duration=15
		ip='10.0.0.146'

	frame_no = -1
	trackingObjs = []
	#creating the video capture for the input video
	if fromLive:
		cap = readUtils.readVideo(fps, duration, ip)
	else:
		cap = cv2.VideoCapture(name + "_orig.mp4")

	#preparing the video out writer
	fourcc = cv2.VideoWriter_fourcc(*'XVID')
	height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
	width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
	fps = cap.get(cv2.CAP_PROP_FPS)
	log.LOG_INFO("FPS:", fps)
	if writeOrig:
		orig = cv2.VideoWriter(name + "_orig.avi", fourcc, fps, (width,height))
	out = cv2.VideoWriter(name + "_out.avi", fourcc, fps, (width,height))

	#Looping through every frame of the video
	while True:
		frame_no += 1
		if (frame_no > 10):
			break
		ret, frame = cap.read()
		if not ret:
			break
		if writeOrig:
			orig.write(frame)
		processFrame(frame, trackingObjs, frame_no % scaling == 0, out, verbose=verbose)
		log.LOG_INFO("\nNext Frame", frame_no+1, "\n")

	cap.release()
	out.release()
	if writeOrig:
		orig.release()

	delete = False
	if delete:
		for trackingObj in trackingObjs:
			databaseUpdate.deleteTrackingObject(trackingObj)
	else:
		log.LOG_INFO("To clear table, execute following statements:\n***")
		for trackedObject in trackingObjs:
			id = str(trackedObject.uuid)[:8]
			query_delete_record = "DELETE FROM cameraRecords WHERE tracking_id=\'" + id + "\'" + ";"
			query_delete_table = "DROP TABLE IF EXISTS obj_" + id + ";"
			print(query_delete_record)
			print(query_delete_table)
		print("***")

if __name__ == '__main__':
	track()

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
