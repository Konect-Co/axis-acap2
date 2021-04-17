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

def genAnalytics():
    outputFile = getOutputFilename()
    #total number people seen every 5 (make 5 a variable?) minutes
    NumPeople = [] 
    #gender, race, age table indicating number of people in past specified amount of time
    intersectionality = np.zeros((2, 5, 5))
    with open(outputFile, 'w') as f:
        #json.dump()
        print("temp")
