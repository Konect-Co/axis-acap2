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
        self.color = genRandomColor()
        self.streakUntracked = 0

        self.age = []
        self.gender = []
        self.race = []

    def updateBox(self, bbox, time=None):
        self.bbox = bbox
        self.latestUpdate = time if time is not None else datetime.datetime.now()
