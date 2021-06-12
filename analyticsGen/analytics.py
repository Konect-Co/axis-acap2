import json
import mysql.connector
import datetime
import numpy as np

#import log

databaseHost = "18.188.84.0"
databaseUser = "ravit"
databasePassword = "mypass123"
database = "spaspect"
databaseConfigFile = "config.json"

#lower_bound
gender_categories = ["Male", "Female", None]
age_categories = ['0-2', '3-9', '10-19', '20-29', '30-39', '40-49', '50-59', '60-69' '70+']
race_categories = ["White", "Black", "Asian", "Latino/Hispanic", "East Asian", "Southeast Asian", "Indian", "Middle Eastern", None]


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

def lonlat2Grid(lat_val, lon_val, lonlat_corners):
    lat_start, lat_end, lon_start, lon_end = lonlat_corners
    grid_val = [
        int((lat_val-lat_start)*float(config["heatMapDimension"][0])/(lat_end-lat_start)),
        int((lon_val-lon_start)*float(config["heatMapDimension"][1])/(lon_end-lon_start))
    ]

    print("[DEBUG]", [lat_val, lon_val], "maps to", grid_val)

    return grid_val

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
        raceCount = [0, 0, 0, 0, 0, 0, 0, 0, 0]
        ageCount = [0, 0, 0, 0, 0, 0, 0, 0, 0]
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
                        if (agePred == age_categories[i]):
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
    intersectionality = np.zeros((2, 8, 8))
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

def computeHeatMap (recentRows, rowInfo):
    heatMap = np.zeros((config["heatMapDimension"][0], config["heatMapDimension"][1]))
    for row in recentRows:
        tracking_id, start_time, end_time, active, race, gender, age = row
        for tracking_elem in rowInfo[tracking_id]:
            _, _, _, _, _, lat_val, lon_val = tracking_elem
            #TODO: Generate the heatmap grid values from lat and lon
            heatmap_x, heatmap_y = lonlat2Grid(lat_val, lon_val, config["lonlatCorners"])
            #heatmap_x = 0, heatmap_y = 0
            heatMap[heatmap_x][heatmap_y] += 1
    return heatMap

def addLonLat(rowInfo):
    lonLatInfo = []

    for key in list(rowInfo.keys()):
        for i in range(len(rowInfo[key])):
            lonLatInfo.append([rowInfo[key][i][5], rowInfo[key][i][6], 0.1])

    return lonLatInfo

def genAnalytics():
    outputFile = getOutputFilename()
    
    analytics = {}

    rowQuery = "select * from cameraRecords where start_time >= Date_sub(now(), interval " + str(frequency) + " hour);"
    cursor.execute(rowQuery)
    recentRows = cursor.fetchall()
    #log.LOG_INFO("Obtained records from past " + str(frequency) + " hour(s)")
    print("Obtained records from past " + str(frequency) + " hour(s)")

    rowInfo = {}
    for row in recentRows:
        tracking_id, _, _ , _, _, _, _ = row
        selectQuery = "select * from obj_" + tracking_id + ";"
        cursor.execute(selectQuery)
        rowInfo[tracking_id] = cursor.fetchall()

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
    analytics["lonlat"] = addLonLat(rowInfo)

    # analytics["heatMap"] = {}
    # analytics["heatMap"]["heatMapDimension"] = config["heatMapDimension"]
    # analytics["heatMap"]["lonlatCorners"] = config["lonlatCorners"]
    # analytics["heatMap"]["value"] = computeHeatMap(recentRows, rowInfo).tolist()
    
    #log.LOG_INFO("Finished computing analytics. Writing to output file.")
    print("Finished computing analytics. Writing to output file.")
    with open(outputFile, 'w') as f:
        json.dump(analytics, f, indent=2)
        foo = 1
    #log.LOG_INFO("Written to output file", outputFile)
    print("Written to output file", outputFile)

if (__name__ == '__main__'):
    genAnalytics()
