# Michel Grimminck 2016, copy freely
import time

from picamera.array import PiRGBArray
from picamera import PiCamera
import MDGImageLib

width = 1920
height = 1080

camera=PiCamera()
camera.resolution = (width, height)
camera.start_preview()

# wait for the camera to settle
time.sleep(2)

rawCapture = PiRGBArray(camera, size=(width,height))
while True:
    camera.capture(rawCapture, format="bgr")
    # focus on center of the image
    sumgrad=MDGImageLib.GradientSum(rawCapture.array,int(0.333*width),int(0.333*height),int(0.666*width),int(0.666*height))
    print (sumgrad)
    rawCapture.truncate(0)
