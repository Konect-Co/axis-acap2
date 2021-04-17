import json
import uuid
import random
import datetime
import cv2
#import js2py
import os

#from Naked.toolshed.shell import execute_js, muterun_js


masterTable = "cameraRecords"

class TrackedObject:
	def __init__(self, tracker, bbox=[-1, -1, -1, -1]):
	    self.tracker = tracker
	    self.uuid = uuid.uuid4()
	    self.latestUpdate = datetime.datetime.now()
	    #(x, y, w, h) format
	    self.bbox = bbox
	    #self.color = genRandomColor()
	    self.streakUntracked = 0

	    self.age = None
	    self.gender = None
	    self.race = None

def getKeysValuesStr(fields):
    keys_str = '('
    values_str = '('

    notFirst = False
    for (key, value) in fields.items():
        if value is None:
            continue
        if notFirst:
            keys_str += ','
            values_str += ','
        keys_str += key
        values_str += '\'' + str(value) + '\''
        if not notFirst:
            notFirst = True

    keys_str += ')'
    values_str += ')'

    return keys_str, values_str


def newTrackingObject(trackedObject):

	sendData = {}

	id = str(trackedObject.uuid)[:8]

    #Query to make a new table corresponding to the current object
	query_makeTable = "CREATE TABLE obj_" + id + " (time TIMESTAMP, pixel_x int, pixel_y int, pixel_w int, pixel_h int, latitude float(25), longitude float(25));"
	sendData["query_makeTable"] = query_makeTable

	#Inserting into the newly created object
	fields = {'time': trackedObject.latestUpdate, 'pixel_x':trackedObject.bbox[0], 'pixel_y':trackedObject.bbox[1], 'pixel_w':trackedObject.bbox[2], 'pixel_h':trackedObject.bbox[3]}
	keys_str, values_str = getKeysValuesStr(fields)
	query_updateTable = "INSERT INTO obj_" + id + " " + keys_str + " VALUES " + values_str + ";"
	sendData["query_updateTable"] = query_updateTable

	#Inserting into the master record with the tracked object
	fields = {'tracking_id': id, 'start_time': trackedObject.latestUpdate, 'end_time': trackedObject.latestUpdate, 'active':'Yes', 'race': trackedObject.race, 'age': trackedObject.age, 'gender': trackedObject.gender}
	keys_str, values_str = getKeysValuesStr(fields)
	query_updateMaster = "INSERT INTO " + masterTable + " " + keys_str + " VALUES " + values_str + ";"
	sendData["query_updateMaster"] = query_updateMaster

	with open("commands.json", "w") as write_file:
		json.dump(sendData, write_file)

def main():
	obj1 = TrackedObject(cv2.TrackerCSRT_create(), [129, 345, 234, 908])
	obj2 = TrackedObject(cv2.TrackerCSRT_create())
	obj3 = TrackedObject(cv2.TrackerCSRT_create(), [34, 21, 34, 44])
	trackedObjects = [obj1, obj2, obj3]

	i = 0
	num = 3
	os.system("sudo node updateDatabase.js")

	while i < num:
		print(i)
		newTrackingObject(trackedObjects[i])
		#success = execute_js('updateDatabase.js')
		i += 1


	print("[INFO] Sent")

if __name__ == '__main__':
	main()