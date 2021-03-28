import cv2

name = "video1"

cap = cv2.VideoCapture(name + ".mp4")
tracker = cv2.TrackerCSRT_create()
ret, frame = cap.read()
#bbox  = cv2.selectROI("tracking", frame)
bbox = (136, 351, 158, 468)
tracker.init(frame, bbox)

cv2.destroyAllWindows()

fps = round(cap.get(cv2.CAP_PROP_FPS))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))

desired_fps = 5
#assuming scaling is a whole number
scaling = round(fps/desired_fps)

fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter(name + "_out.avi", fourcc, desired_fps, (width,height))

#of the original video
frame_no = 1
while True:
	ret, frame = cap.read()
	if not ret:
		break
	if frame_no % scaling == 0:
		success, box = tracker.update(frame)
		if success:
			(x, y, w, h) = [int(v) for v in box]
			_ = cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
			out.write(frame)
		else:
			print("Failed at frame", frame_no)
			break
	frame_no += 1

cap.release()
out.release()
