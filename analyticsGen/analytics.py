import json
import mysql.connector
import datetime
import numpy as np

import log

databaseHost = "18.188.84.0"
databaseUser = "ravit"
databasePassword = "mypass123"
database = "spaspect"
databaseConfigFile = "config.json"

#lower_bound
gender_categories = ["male", "female"]
age_categories = [0, 10, 20, 40, 65]
race_categories = ["White", "Black", "Asian", "Indian", "Other"]


def getOutputFilename():
    return "analyticsRecords/" + "".join(str(datetime.datetime.now()).split(' ')) + ".json"

with open(databaseConfigFile, 'r') as f:
    config = json.load(f)

#frequency with which updating occurs
frequency = float(config["frequency"])

my_database = mysql.connector.connect(
    host=databaseHost,
    user=databaseUser,
    password=databasePassword,
    database=database
)
cursor = my_database.cursor()

def computeIntersectionality(recentRows):
    #gender, race, age table indicating number of people in past specified amount of time
    intersectionality = np.zeros((2, 5, 5))
    for row in recentRows:
        tracking_id, start_time, end_time, active, race, gender, age = row
        
        if (race is None or gender is None or age is None):
            continue

        gender_category = gender_categories.index(gender)
        race_category = race_categories.index(race)
        age_category = 0
       
        for i in range(1, len(age_categories)):
            if (age_category >= age_categories[i]):
                age_category = age_categories[i]
            else:
                break

        intersectionality[gender_category][race_category][age_category] += 1
    return intersectionality 

def genAnalytics():
    outputFile = getOutputFilename()
    
    analytics = {}

    rowQuery = "select * from cameraRecords where start_time >= Date_sub(now(), interval " + str(frequency) + " hour);"
    cursor.execute(rowQuery)
    recentRows = cursor.fetchall()
    log.LOG_INFO("Obtained records from past " + str(frequency) + " hour(s)")
    
    analytics["numPeople"] = len(recentRows)
    analytics["intersectionality"] = computeIntersectionality(recentRows).tolist()
    
    log.LOG_INFO("Finished computing analytics. Writing to output file.")

    with open(outputFile, 'w') as f:
        json.dump(analytics, f, indent=2)
        foo = 1
    log.LOG_INFO("Written to output file", outputFile)

if (__name__ == '__main__'):
    genAnalytics()
