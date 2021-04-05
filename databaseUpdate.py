import mysql.connector
import log

my_database = mysql.connector.connect(
    host="18.188.84.0",
    user="ravit",
    password="mypass123",
    database="spaspect"
)
mycursor = my_database.cursor()

log.LOG_INFO("Connected to:", my_database.get_server_info())

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
    id = str(trackedObject.uuid)[:8]

    """
    query_delete_record = "DELETE FROM cameraRecords WHERE tracking_id=\'" + id + "\'"
    mycursor.execute(query_delete_record)
    """

    query_alter_activity = "UPDATE cameraRecords SET active=\'No\' WHERE tracking_id=\'" + id + "\'"
    mycursor.execute(query_alter_activity)

    query_delete_table = "DROP TABLE IF EXISTS obj_" + id
    mycursor.execute(query_delete_table)
    
    my_database.commit()

def updateTrackingObject(trackedObject):
    id = str(trackedObject.uuid)[:8]

    fields = {"end_time":trackedObject.latestUpdate}
    updateFieldsStr = getUpdateKeysValueStr(fields)
    query_update_record = "UPDATE cameraRecords SET " + updateFieldsStr + " WHERE tracking_id=\'" + id + "\'"
    mycursor.execute(query_update_record)

    #Inserting into the tracked object table
    fields = {'time': trackedObject.latestUpdate, 'pixel_x':trackedObject.bbox[0], 'pixel_y':trackedObject.bbox[1], 'pixel_w':trackedObject.bbox[2], 'pixel_h':trackedObject.bbox[3]}
    keys_str, values_str = getKeysValuesStr(fields)
    query_updateTable = "INSERT INTO obj_" + id + " " + keys_str + " VALUES " + values_str
    log.LOG_INFO("Update info is " + query_updateTable)
    mycursor.execute(query_updateTable)

    my_database.commit()

def newTrackingObject(trackedObject):

    id = str(trackedObject.uuid)[:8]

    #Query to make a new table corresponding to the current object
    query_makeTable = "CREATE TABLE obj_" + id + " (time TIMESTAMP, pixel_x int, pixel_y int, pixel_w int, pixel_h int, latitude float(25), longitude float(25));"
    mycursor.execute(query_makeTable)

    #Inserting into the newly created object
    fields = {'time': trackedObject.latestUpdate, 'pixel_x':trackedObject.bbox[0], 'pixel_y':trackedObject.bbox[1], 'pixel_w':trackedObject.bbox[2], 'pixel_h':trackedObject.bbox[3]}
    keys_str, values_str = getKeysValuesStr(fields)
    query_updateTable = "INSERT INTO obj_" + id + " " + keys_str + " VALUES " + values_str
    log.LOG_INFO("Update info is " + query_updateTable)
    mycursor.execute(query_updateTable)
    
    #Inserting into the master record with the tracked object
    fields = {'tracking_id': id, 'start_time': trackedObject.latestUpdate, 'end_time': trackedObject.latestUpdate, 'active':'Yes'}
    keys_str, values_str = getKeysValuesStr(fields)
    query_updateMaster = "INSERT INTO " + masterTable + " " + keys_str + " VALUES " + values_str;
    mycursor.execute(query_updateMaster)

    my_database.commit()

    log.LOG_INFO(mycursor.rowcount, "record(s) affected")

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
    for trackedObject in trackedObjects:
        id = str(trackedObject.uuid)[:8]
        countQuery = "SELECT count(*) FROM cameraRecords WHERE tracking_id = \'" + id + "\'"
        mycursor.execute(countQuery)

        countResultRaw = mycursor.fetchall()
        countResult = countResultRaw[0][0]
        my_database.commit()
        if countResult == 0:
            newTrackingObject(trackedObject)
        elif countResult == 1:
            updateTrackingObject(trackedObject)
        else:
            log.LOG_ERR("Not expecting countResult to have value " + str(countResult))

