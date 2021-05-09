import numpy as np
import cv2
import json
import math

"""
Create an object for converting pixels to geographic coordinates,
using four points with known locations which form a quadrilteral in both planes
"""
class PixelMapper(object):
	def __init__(self, pixel_array, lonlat_array, lonlat_center):
		#converting pixel and lonlat array to numpy array
		pixel_array = np.array(pixel_array)
		lonlat_array = np.array(lonlat_array)

		#identifying shape of arrays
		assert pixel_array.shape==(4,2), "Need (4,2) input array"
		assert lonlat_array.shape==(4,2), "Need (4,2) input array"

		#Calculates a perspective transform from four pairs of the corresponding points
		self.M = cv2.getPerspectiveTransform(np.float32(pixel_array),np.float32(lonlat_array))
        

	def pixel_to_lonlat(self, pixel):
		"""
		Convert a set of pixel coordinates to lon-lat coordinates
		"""
		if type(pixel) != np.ndarray:
			pixel = np.array(pixel).reshape(1,2)
		assert pixel.shape[1]==2, "Need (N,2) input array"
		#Joining the sequence of arrays along an existing axis
		pixel = np.concatenate([pixel, np.ones((pixel.shape[0],1))], axis=1)
		#calculating dot product of M and pixel.T, which is the lonlat coordinates
		lonlat = np.dot(self.M,pixel.T)
		#returning lonlat coordinates
		return (lonlat[:2,:]/lonlat[2,:]).T.tolist()[0]