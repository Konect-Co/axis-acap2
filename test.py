from tensorflow.keras.models import load_model
from mtcnn.mtcnn import MTCNN
import numpy as np
import cv2

def interpret_demographics_label(age_label, gender_label, race_label):
	races = ["White", "Black", "Asian", "Indian", "Other"]
	age = int(age_label[0]*116)
	gender = "male" if gender_label[0]<0.5 else "female"
	#print("age_label:", age_label)
	#print("Age:",age)
	#print(races, race_label.flatten())
	#print(gender_label[0])
	race = races[np.argmax(race_label.flatten())]

	return age, gender, race

def preprocess_image(image_orig):
	image = cv2.resize(image_orig, (150, 150))
	image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
	image = np.expand_dims(image, 0).astype(np.float16)
	image = image/255
	return image


demographicsDetector = load_model("KonectDemographics.h5")
img = cv2.imread("boy5.png")
image = preprocess_image(img)
prediction = demographicsDetector.predict(image)
age, gender, race = interpret_demographics_label(prediction[0][0], prediction[1][0], prediction[2][0])

print("Age: ", age)
print("Gender: ", gender)
print("Race: ", race)
