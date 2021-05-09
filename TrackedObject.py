import uuid
import random
import datetime

def genRandomColor():
    return (int(random.random()*255), int(random.random()*255), int(random.random()*255))

class TrackedObject:
    def __init__(self, tracker, bbox=[-1, -1, -1, -1]):
        self.tracker = tracker
        self.uuid = uuid.uuid4()
        self.latestUpdate = datetime.datetime.now()
        #(x, y, w, h) format
        self.bbox = bbox
        self.lonlat = [None, None]
        self.color = genRandomColor()
        self.streakUntracked = 0

        self.age = None
        self.gender = None
        self.race = None

    def getBbox(self):
        return self.bbox

    def updateBox(self, bbox, pm, time=None):
        self.bbox = bbox
        self.latestUpdate = time if time is not None else datetime.datetime.now()
        updatePixel = [bbox[1] + bbox[3], bbox[0] + bbox[2]/2]
        self.lonlat = pm.pixel_to_lonlat(updatePixel)[0]

    def updateRace(self, race):
        self.race = race

    def updateGender(self, gender):
        self.gender = gender

    def updateAge(self, age):
        self.age = age