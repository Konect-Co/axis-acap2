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
gender_categories = ["male", "female", None]
age_categories = [0, 10, 20, 40, 65]
race_categories = ["White", "Black", "Asian", "Indian", "Other", None]


def getOutputFilename():
    return "analyticsRecords/" + "".join(str(datetime.datetime.now()).split(' ')) + ".json"

with open(databaseConfigFile, 'r') as f:
    config = json.load(f)

#frequency with which updating occurs
frequency = float(config["frequency"])
numPeopleInterval = float(config["numPeopleInterval"])

my_database = mysql.connector.connect(
    host=databaseHost,
    user=databaseUser,
    password=databasePassword,
    database=database
)
cursor = my_database.cursor()

def computeNumPeople(recentRows):
    recentRows.sort(key=lambda row:row[1])
    
    numPeopleStartTime = recentRows[0][1]
    endTime = recentRows[-1][1]
    currTime = numPeopleStartTime

    numPeopleTotal = []
    numPeopleByGender = []
    numPeopleByRace = []
    numPeopleByAge = []

    while (currTime < endTime):
        currCount = 0
        genderCount = [0, 0, 0]
        raceCount = [0, 0, 0, 0, 0, 0]
        ageCount = [0, 0, 0, 0, 0, 0]
        upperBound = currTime + datetime.timedelta(0,numPeopleInterval*60)

        for recentRow in recentRows:
            entryTime = recentRow[1]
            exitTime = recentRow[2]

            if (entryTime < upperBound and exitTime > currTime):
                currCount += 1

                raceCount[race_categories.index(recentRow[4])] += 1
                genderCount[gender_categories.index(recentRow[5])] += 1
                
                agePred = recentRow[6]
                age_category = -1
                if (agePred is not None):
                    for i in range(1, len(age_categories)):
                        if (agePred >= age_categories[i]):
                            age_category = i
                        else:
                            break
                ageCount[age_category] += 1

        numPeopleTotal.append(currCount)
        numPeopleByGender.append(genderCount)
        numPeopleByRace.append(raceCount)
        numPeopleByAge.append(ageCount)

        currTime = upperBound

    numPeopleVal = {
        "numPeopleTotal": numPeopleTotal,
        "numPeopleByGender": numPeopleByGender,
        "numPeopleByRace": numPeopleByRace,
        "numPeopleByAge": numPeopleByAge
    }
    return str(numPeopleStartTime), numPeopleInterval, numPeopleVal

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
            if (age >= age_categories[i]):
                age_category = i
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
    
    numPeopleStartTime, numPeopleFrequency, numPeopleVal = computeNumPeople(recentRows)
    analytics["numPeople"] = {}
    analytics["numPeople"]["numPeopleStartTime"] = numPeopleStartTime
    analytics["numPeople"]["numPeopleInterval"] = numPeopleInterval
    analytics["numPeople"]["categories"] = {
        "genderCategories": gender_categories,
        "raceCategories": race_categories,
        "ageCategories": age_categories
    }
    analytics["numPeople"]["numPeopleVal"] = numPeopleVal

    analytics["intersectionality"] = computeIntersectionality(recentRows).tolist()
    
    log.LOG_INFO("Finished computing analytics. Writing to output file.")

    with open(outputFile, 'w') as f:
        json.dump(analytics, f, indent=2)
        foo = 1
    log.LOG_INFO("Written to output file", outputFile)

if (__name__ == '__main__'):
    genAnalytics()
