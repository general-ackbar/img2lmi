This is a C++ version of img2lmi. A small commandline utility to convert bitmap images into lossless matrix images.

LMI is a very simple image format that stores uncompressed image data with a minimal header. It's main purpose is to supply easy image implementation for embedded devices and microcontrollers. E.g the rgb565 format stores the image data as a 16-bit that can readily be loaded directly into most TFT-libraries

Name
img2lmi - encode bitmap images to lossless matrix images (.lmi)

Synopsis
img2lmi -i FILE (options)

Description
img2lmi encodes bitmap images to lossless matrix images and outputs them as .lmi files.

img2lmi can load the most widespread image bitmap formats: PNG, JPEG, GIF, PNG, BMP etc. The default pixel format is rgb565 (note: many arduino/esp8266 etc libraries uses rgb565).

Options
-o Name of the output file. If none is specified the input file's name is used with the .lmi extension
-a Appends the input image data to an existing lmi-file when creating animations. If no output file is specified it will create a new file named after the input file
-r (fps) Set the frame rate. Specify this for the first frame when creating animations -h Display help message and exit.
-b Sets the bits per color. Options are 32 (rgba), 24 (rgb888), 16 (rgb565), 8 (rgb332), 1 (binary) or 0 (vertically ordered binary. Used for e.g. Nokia 3310 displays)
-q Be quiet
-x Invert 1-bit images
-n Don't dither 1-bit images

Examples
Create a lossless matrix image named hello.lmi using 32-bits to represent a full palette including alpha channel

img2lmi -i hello.jpg -b 24

Create a lossless matrix image named bar.lmi using a 16-bits colorspace

img2lmi -i foo.jpg -o bar.lmi

Create an inverted and non-dithered 1-bit black and white file named logo.lmi to use for the PCD8544 LCD-controller

img2lmi -i logo.png -b 0 -x -n 

 

Create an animation with 4 frames at 10 fps and 16-bit colorspace

img2lmi -i frame0.jpg -f 10 -o animation.lmi

img2lmi -i frame1.jpg -a -o animation.lmi

img2lmi -i frame2.jpg -a -o animation.lmi

img2lmi -i frame3.jpg -a -o animation.lmi