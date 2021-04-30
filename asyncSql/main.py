import json
import uuid
import random
import datetime
import cv2
#import js2py
import os
import requests
import json

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

	return sendData

def selectCommands(trackedObject):
	select_data = {}

	id = str(trackedObject.uuid)[:8]
	countQuery = "SELECT count(*) FROM cameraRecords WHERE tracking_id = \'" + id + "\'"
	select_data["countQuery"] = countQuery

	return select_data


def main():

	url = 'http://localhost:3000'

	obj1 = TrackedObject(cv2.TrackerCSRT_create(), [129, 345, 234, 908])
	obj2 = TrackedObject(cv2.TrackerCSRT_create())
	obj3 = TrackedObject(cv2.TrackerCSRT_create())
	obj4 = TrackedObject(cv2.TrackerCSRT_create())
	obj5 = TrackedObject(cv2.TrackerCSRT_create())
	obj6 = TrackedObject(cv2.TrackerCSRT_create())
	obj7 = TrackedObject(cv2.TrackerCSRT_create())
	obj8 = TrackedObject(cv2.TrackerCSRT_create())
	obj9 = TrackedObject(cv2.TrackerCSRT_create(), [34, 21, 34, 44])
	obj10 = TrackedObject(cv2.TrackerCSRT_create(), [34, 21, 34, 44])

	trackedObjects = [obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10]

	i = 0
	num = 10

	while i < num:
		print(i)
		data = selectCommands(trackedObjects[i])
		x = requests.post(url, data=data)
		selectRes = json.loads(x.text)
		print(selectRes[0]["count(*)"])
		i += 1

	#print(x.text)

	print("[INFO] Sent")

	print("To clear table, execute following statements:\n***")
	for trackedObject in trackedObjects:
		id = str(trackedObject.uuid)[:8]
		query_delete_record = "DELETE FROM cameraRecords WHERE tracking_id=\'" + id + "\'" + ";"
		query_delete_table = "DROP TABLE IF EXISTS obj_" + id + ";"
		print(query_delete_record)
		print(query_delete_table)

if __name__ == '__main__':
	main()