# raspberryFocus
Small program to focus the lens of a raspberry pi camera without a monitor attached

This program measures how sharp is the image of a raspberry camera, and outouts this to the console.

MeasureFocus tries to give a measure of sharpness of the raspberry camera. This enables you to focus the raspberry
camera without the need for a live-view.

How it works
============
MeasureFocus measures the sharpness by summing all the gradients in the image. This is programmed in python for flexibility,
combined with C for speed.

How to install:
===============
1) copy all files to some directory on your raspberry
2) sudo python MDGImageLibSetup.py build
3) sudo python MDGImageLibSetup.py install

How to use:
===========
1) Aim your camera at your target
2) Run 'python focus.py'
3) Change the focus until you find the highest number.
   There are a few caveats:
   - Try not to change the aim of the camera. The sharpness number is dependend on what it is watching
   - Make sure the lighting conditions do not change while aiming
   - There is always some randomness in the numbers; this is because of noise in the camera

Notes
=====
By default, the program looks at the center of the pi camera. You can change this by alterting the source code in focus.py

0.0  0.33   0.66   1

......|......|......
......|......|......
------+------+------ 0.33
......|xxxxxx|......
......|xxxxxx|......
------+------+------ 0.66
......|......|......
......|......|...... 1.00
