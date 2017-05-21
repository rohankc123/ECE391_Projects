transparency = .3
colors = [
	[229,255,204],
	[204,255,255],
	[255,204,205],
	[224,204,255],
	[255,255,255],
	[0,153,0],
	[0,76,153],
	[153,0,0],
	[76,0,153],
	[0,0,0],
]


# for x in range(len(colors)):
    # # while colors[x][0]*transparency >= 255:
        # # transparency -= .1
    # # while colors[x][1]*transparency >= 255:
        # # transparency -= .1
    # # while colors[x][2]*transparency >= 255:
        # # transparency -= .1
    # colors[x][0]=(0x3f+colors[x][0])/2
    # colors[x][1]=(0x3f+colors[x][1])/2
    # colors[x][2]=(0x3f+colors[x][2])/2
# for color in colors:
    # print("{" + str(int(color[0])) + ", " + str(int(color[1])) + ", " + str(int(color[2])) + "}, ")

for x in range(len(colors)):
    # while colors[x][0]*transparency >= 255:
        # transparency -= .1
    # while colors[x][1]*transparency >= 255:
        # transparency -= .1
    # while colors[x][2]*transparency >= 255:
        # transparency -= .1
    colors[x][0]=colors[x][0]&0x3F
    colors[x][1]=colors[x][1]&0x3F
    colors[x][2]=colors[x][2]&0x3F

for x in range(len(colors)):
    # while colors[x][0]*transparency >= 255:
        # transparency -= .1
    # while colors[x][1]*transparency >= 255:
        # transparency -= .1
    # while colors[x][2]*transparency >= 255:
        # transparency -= .1
    colors[x][0]=(0x3f+colors[x][0])/2
    colors[x][1]=(0x3f+colors[x][1])/2
    colors[x][2]=(0x3f+colors[x][2])/2
for color in colors:
    print("{" + str(int(color[0])) + ", " + str(int(color[1])) + ", " + str(int(color[2])) + "}, ")
