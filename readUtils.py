def readVideo(fps, duration, ip):
	log.LOG_INFO("Starting Read")
	wget_command = "wget -q --user root --password pass123 \"http://" + ip + "/mjpg/video.mjpg?fps=" + str(fps) + "&duration=" + str(duration) + "\" -O video.mjpeg"
	os.system(wget_command)
	cap = cv2.VideoCapture("video.mjpeg")
	log.LOG_INFO("Ending Read")
	return cap

def readImage(fps, duration, ip):
	cap = readVideo(fps, duration, ip)
	res, image = cap.read()
	image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
	if not res:
		log.LOG_ERR("Failed to read video.mjpeg")
		return
	return image