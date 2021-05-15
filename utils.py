import cv2
import numpy as np

#box format: [x, y, height, width] where (x,y) is the top left corner
#this function computes the area of a box
def computeArea(box):
	box_reformed = [box[0], box[1], box[0]+box[2], box[1]+box[3]]
	return (box_reformed[2]-box_reformed[0])*(box_reformed[3]-box_reformed[1])

#this function computes the intersection between two boxes
def computeIntersection(boxA, boxB):
	boxA_reformed = [boxA[0], boxA[1], boxA[0]+boxA[2], boxA[1]+boxA[3]]
	boxB_reformed = [boxB[0], boxB[1], boxB[0]+boxB[2], boxB[1]+boxB[3]]

	#cases to return 0
	if (boxB_reformed[0]>boxA_reformed[2] or boxA_reformed[0]>boxB_reformed[2] or boxB_reformed[1]>boxA_reformed[3] or boxA_reformed[1]>boxB_reformed[3]):
		return 0

	#determining the X and Y values
	xVals = [boxA_reformed[0], boxA_reformed[2], boxB_reformed[0], boxB_reformed[2]]
	yVals = [boxA_reformed[1], boxA_reformed[3], boxB_reformed[1], boxB_reformed[3]]
	#arranging from least to greatest
	xVals.sort()
	yVals.sort()
	
	#returning intersection
	intersection = (xVals[2]-xVals[1])*(yVals[2]-yVals[1])
	return intersection

#this function computes the IoU between two boxes
def computeIOU (boxA, boxB):

	#computing intersection from above method
	intersection = computeIntersection(boxA, boxB)
	#computing area from above method
	union = computeArea(boxA) + computeArea(boxB) - intersection
	iou = intersection/union

	#returning IOU
	return iou

#this function computes Intersection over Area of A
def computeIOA (boxA, boxB):
	intersection = computeIntersection(boxA, boxB)
	aArea = computeArea(boxA)
	ioa = intersection/aArea

	#returning IOA
	return ioa

def xyxy2yxhw(xyxy):
	return [xyxy[1], xyxy[0], xyxy[3]-xyxy[1], xyxy[2]-xyxy[0]]

def interpret_demographics_label(age_label, gender_label, race_label):
	races = ["White", "Black", "Asian", "Indian", "Other"]
	age = int(age_label[0]*116)
	gender = "male" if gender_label[0]<0.5 else "female"
	race = races[np.argmax(race_label.flatten())]

	return age, gender, race

def preprocess_image(image_orig):
	image = cv2.resize(image_orig, (150, 150))
	image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
	image = np.expand_dims(image, 0).astype(np.float16)
	image = image/255
	return image