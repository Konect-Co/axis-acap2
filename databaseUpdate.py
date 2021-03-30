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
