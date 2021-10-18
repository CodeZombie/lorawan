from PIL import Image
import random

image = Image.open('topology.png') # Can be many different formats.
pixels = image.load()

node_location_file = open("node_locations.dat","w")
node_locations = []
for x in range(image.size[0]):
    for y in range(image.size[1]):
        if pixels[x,y] == (0, 0, 0, 255):
            node_locations.append([x,y])

random.shuffle(node_locations)

for location in node_locations:
    node_location_file.write(str(location[0]) + "," + str(location[1]) + ",0\n")

print("Found " + str(len(node_locations)) + " end-nodes")



node_location_file.close()
#print pix[x,y]  # Get the RGBA Value of the a pixel of an image
#pix[x,y] = value  # Set the RGBA Value of the image (tuple)
#im.save('alive_parrot.png')  # Save the modified pixels as .png