#!/usr/bin/python

from PIL import Image
from string import Template
import sys

ROWSIZE = 20

def swap32(i):
	return struct.unpack("<I", struct.pack(">I", i))[0]

def swapi(i):
	h = int(i)>>8;
	l = int(i) & 0xff;
	return l<<8 | h;

# Convert 24 bit RGB to 16 bit 565 format
def to565(pixel) :
    r = pixel[0]
    g = pixel[1]
    b = pixel[2]
    return swapi((int(b*31/255) << 11) | (int(g*63/255) << 5) | (int(r*31/255)))
#    return (((b & 0x00F8) << 8) | ((g & 0x00FC) << 3) | ((r & 0x00F8) >> 3)


# MAIN
# MAIN
# MAIN

# Load image from command line
if len(sys.argv) < 2:
    print 'No input file provided'
    exit()

# Open the source image
imgi = Image.open(sys.argv[1])
img = imgi.convert('RGB')
imgdata = list(img.getdata())

# Get the name of the file
imgname = sys.argv[1].split('.')[0]

# Print out a bit of info
print 'Image name:   {0}'.format(imgname)
print 'Image size:   {0}'.format(img.size)
print 'Image format: {0}'.format(img.format)

# Open the template
templatefile = open("template_c.txt", "r")
template = Template(templatefile.read())

# Build the template parameter list
data = {}
data['imgname'] = imgname
data['imgnamecaps'] = imgname.upper()
data['imglen'] = img.size[0] * img.size[1]
data['imgdata'] = ',\n\t'.join([', '.join(['0x{:04X}'.format(to565(x)) for x in imgdata[y : y + ROWSIZE]]) for y in xrange(0, len(imgdata), ROWSIZE)])
data['imglenw'] = img.size[0]
data['imglenh'] = img.size[1]


# Open the the text file
outputfile = open(imgname + ".c", "w")
outputfile.write(template.substitute(data))
outputfile.close()

