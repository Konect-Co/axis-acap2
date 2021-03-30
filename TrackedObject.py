import uuid
import random

def genRandomColor():
	return (int(random.random()*255), int(random.random()*255), int(random.random()*255))

class TrackedObject:
	def __init__(self, tracker, bbox=None):
		self.tracker = tracker
		self.uuid = uuid.uuid4()
		self.bbox = bbox
		self.color = genRandomColor()

	def updateBox(self, bbox):
		self.bbox = bbox
