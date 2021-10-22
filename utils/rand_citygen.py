import random
import math

node_location_file = open("node_locations.dat","w")
radius = 5000
for i in range(1024):
    r = radius * math.sqrt(random.uniform(0,1))
    theta = random.uniform(0,1) * 2 * math.pi
    x = math.floor(0 + r * math.cos(theta))
    y = math.floor(0 + r * math.sin(theta))
    node_location_file.write(str(x) + "," + str(y) + ",0\n")

node_location_file.close()
