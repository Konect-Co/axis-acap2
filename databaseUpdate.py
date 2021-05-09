import log
import requests
import json

masterTable = "cameraRecords"

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

def getUpdateKeysValueStr(fields):
    notFirst = False
    result = ""
    for (key, value) in fields.items():
        if value is None:
            continue
        if notFirst:
            result += ", "
        result += key + "=" + '\'' + str(value) + '\''
        if not notFirst: 
            notFirst = True
    return result

def deleteTrackingObject(trackedObject):
    delete_data = {}
    id = str(trackedObject.uuid)[:8]

    """
    query_delete_record = "DELETE FROM cameraRecords WHERE tracking_id=\'" + id + "\'"
    mycursor.execute(query_delete_record)
    """

    query_alter_activity = "UPDATE cameraRecords SET active=\'No\' WHERE tracking_id=\'" + id + "\'"
    delete_data["query_alter_activity"] = query_alter_activity

    query_delete_table = "DROP TABLE IF EXISTS obj_" + id
    delete_data["query_delete_table"] = query_delete_table
    
    return delete_data

def updateTrackingObject(trackedObject):
    update_data = {}

    id = str(trackedObject.uuid)[:8]

    fields = {"end_time":trackedObject.latestUpdate, "age":trackedObject.age, "gender": trackedObject.gender, "race": trackedObject.race}
    updateFieldsStr = getUpdateKeysValueStr(fields)
    query_update_record = "UPDATE cameraRecords SET " + updateFieldsStr + " WHERE tracking_id=\'" + id + "\'"
    update_data["query_update_record"] = query_update_record

    #Inserting into the tracked object table
    fields = {'time': trackedObject.latestUpdate, 'pixel_x':trackedObject.bbox[0], 'pixel_y':trackedObject.bbox[1], 'pixel_w':trackedObject.bbox[2], 'pixel_h':trackedObject.bbox[3], 'lat': trackedObject.lonlat[0], 'lon': trackedObject.lonlat[1]}
    keys_str, values_str = getKeysValuesStr(fields)
    query_updateTable = "INSERT INTO obj_" + id + " " + keys_str + " VALUES " + values_str
    log.LOG_INFO("Update info is " + query_updateTable)
    update_data["query_updateTable"] = query_updateTable

    return update_data

def newTrackingObject(trackedObject):
    new_data = {}

    id = str(trackedObject.uuid)[:8]

    #Query to make a new table corresponding to the current object
    query_makeTable = "CREATE TABLE obj_" + id + " (time TIMESTAMP, pixel_x int, pixel_y int, pixel_w int, pixel_h int, latitude float(25), longitude float(25), lat float(10), lon float(10));"
    new_data["query_makeTable"] = query_makeTable

    #Inserting into the newly created object
    fields = {'time': trackedObject.latestUpdate, 'pixel_x':trackedObject.bbox[0], 'pixel_y':trackedObject.bbox[1], 'pixel_w':trackedObject.bbox[2], 'pixel_h':trackedObject.bbox[3], 'lat': trackedObject.lonlat[0], 'lon': trackedObject.lonlat[1]}
    keys_str, values_str = getKeysValuesStr(fields)
    query_updateTable = "INSERT INTO obj_" + id + " " + keys_str + " VALUES " + values_str
    log.LOG_INFO("Update info is " + query_updateTable)
    new_data["query_updateTable"] = query_updateTable
    
    #Inserting into the master record with the tracked object
    fields = {'tracking_id': id, 'start_time': trackedObject.latestUpdate, 'end_time': trackedObject.latestUpdate, 'active':'Yes', 'race': trackedObject.race, 'age': trackedObject.age, 'gender': trackedObject.gender}
    keys_str, values_str = getKeysValuesStr(fields)
    query_updateMaster = "INSERT INTO " + masterTable + " " + keys_str + " VALUES " + values_str;
    new_data["query_updateMaster"] = query_updateMaster


    return new_data

"""
trackedObjects is a list of TrackedObject instances.
"""
def addToDatabase(trackedObjects):
    """
    Obtain active rows from the database
    for every TrackedObject curr_obj present in the database:
        if curr_obj not present in trackedObj:
            set TrackedObject status in database to inactive
        else:
            update the database row with the new bounding box
    for every TrackedObject curr_obj present in trackedObjects:
        if curr_obj not present in the database:
            add new entry to database
    """
    url = 'http://localhost:3000'
    main_data = {}

    for trackedObject in trackedObjects:
        id = str(trackedObject.uuid)[:8]
        countQuery = "SELECT count(*) FROM cameraRecords WHERE tracking_id = \'" + id + "\'"
        main_data["countQuery"] = countQuery

        x = requests.post(url, data=main_data)
        selectRes = json.loads(x.text)
        countResult = selectRes[0]["count(*)"]

        if countResult == 0:
            data = newTrackingObject(trackedObject)
            x = requests.post(url, data=data)
        elif countResult == 1:
            data = updateTrackingObject(trackedObject)
            x = requests.post(url, data=data)
        else:
            log.LOG_ERR("Not expecting countResult to have value " + str(countResult))