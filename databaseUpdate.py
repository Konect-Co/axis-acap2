import mysql.connector

import log

my_database = mysql.connector.connect(
   host="localhost",
   user="root",
   password="mypass123",
   database="retail"
)

log.LOG_INFO("Connected to:", my_database.get_server_info())

"""
mycursor = my_database.cursor()

query = "UPDATE peopleTracking SET active = %s WHERE tracking_id = %s"
values = ('No', '33487cd4')

mycursor.execute(query, values)
my_database.commit()

log.LOG_INFO(mycursor.rowcount, "record(s) affected")
"""

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
