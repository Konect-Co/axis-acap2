import mysql.connector
import log
from sshtunnel import SSHTunnelForwarder

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


my_database = mysql.connector.connect(
    host="18.188.84.0",
    user="santript",
    password="mypass123",
    database="spaspect"
)

print("Connected to:", my_database.get_server_info())

mycursor = my_database.cursor()

query = "UPDATE cameraRecords SET active = %s WHERE tracking_id = %s"
values = ('No', '52e75c81')

mycursor.execute(query, values)
my_database.commit()

log.LOG_INFO(mycursor.rowcount, "record(s) affected")
