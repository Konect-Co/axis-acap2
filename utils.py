import numpy as np
import torchvision
import cv2

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

def interpret_demographics_label(outputs):
	outputs = outputs.cpu().detach().numpy()
	outputs = np.squeeze(outputs)

	race_outputs = outputs[:7]
	gender_outputs = outputs[7:9]
	age_outputs = outputs[9:18]

	race_score = np.exp(race_outputs) / np.sum(np.exp(race_outputs))
	gender_score = np.exp(gender_outputs) / np.sum(np.exp(gender_outputs))
	age_score = np.exp(age_outputs) / np.sum(np.exp(age_outputs))

	race_pred = np.argmax(race_score)
	gender_pred = np.argmax(gender_score)
	age_pred = np.argmax(age_score)

	gender_labels = ['Male', 'Female']
	race_labels = ['White', 'Black', 'Latino/Hispanic', 'East Asian', 'Southeast Asian', 'Indian', 'Middle Eastern']
	age_labels = ['0-2', '3-9', '10-19', '20-29', '30-39', '40-49', '50-59', '60-69' '70+']

	return gender_labels[gender_pred], race_labels[race_pred], age_labels[age_pred]

def preprocess_demographics_img(image_name, device):
	trans = torchvision.transforms.Compose([
	    torchvision.transforms.ToPILImage(),
	    torchvision.transforms.Resize((224, 224)),
	    torchvision.transforms.ToTensor(),
	    torchvision.transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
	])

	#image = dlib.load_rgb_image(img_name)
	image = cv2.cvtColor(image_name, cv2.COLOR_BGR2RGB)
	image = trans(image_name)
	image = image.view(1, 3, 224, 224)  # reshape image to match model dimensions (1 batch size)
	image = image.to(device)

	return image