

import numpy as np
import cv2
import matplotlib.pyplot as plt
from PIL import Image
import math

filename = 'test/ball'
image_file = filename + '.png'
output_file = filename + 'Deformed.png'
contour_file = filename + 'Contours.png'
edge_file = filename + 'Edged.png'

p = []
q = []

count1 = 0
count2 = 0

x1_values = []
y1_values = []

x2_values = []
y2_values = []

# Reference: https://www.thepythoncode.com/article/canny-edge-detection-opencv-python
# # https://docs.opencv.org/master/d4/d73/tutorial_py_contours_begin.html
# # https://stackoverflow.com/questions/34884779/whats-a-simple-way-of-warping-an-image-with-a-given-set-of-points
# # https://www.owlnet.rice.edu/~elec539/Projects97/morphjrks/warp.html

# function to interact with application to select the points clicked on the image 
def click_event(event, x, y, flags, params):
    global count1
    global count2
    global p
    global q
    global x1_values
    global y1_values
    global x2_values
    global y2_values
    # checking for left mouse clicks
    if event == cv2.EVENT_LBUTTONDOWN:
        # displaying the coordinates on the Shell
        print(x, ' ', y)

        # displaying the points on the image window
        cv2.circle(A, (x,y), radius=0, color=(0, 50, 255),thickness=2)
        cv2.imshow('image', A)
        
        #draw_points.append([x,y])
        p.append([x,y])
        x1_values.append(x)
        y1_values.append(y)
        count1 += 1

    # checking for right mouse clicks     
    if event==cv2.EVENT_RBUTTONDOWN:
  
        # displaying the coordinates on the Shell
        print(x, ' ', y)
  
        # displaying the points on the image window
        b = A[y, x, 0]
        g = A[y, x, 1]
        r = A[y, x, 2]
        cv2.circle(A, (x,y), radius=0, color=(255,0,0),thickness=2)
        cv2.imshow('image', A)
        
        q.append([x,y])
       
        x2_values.append(x)
        y2_values.append(y)
        count2 += 1
        print(count1, count2)

    # checking for middle mouse clicks     
    if event==cv2.EVENT_MBUTTONDOWN:

        #require at least 3 points to create a curve
        if(count2 >= 3 and count1 >= 3 and count1 is count2):
            print(count1, count2)
            p_list = p
            p = np.array(p)
            q = np.array(q)
            k = len(p)

            drawCurve(p,q)
            points = []
            for i in range(count1):
                point = (x1_values[i],y1_values[i],x2_values[i],y2_values[i])
                points.append(point)

            count1 = 0
            count2 = 0

            #apply warping function to hte image using the user-specified feature curve
            warpImage(points)

# create curve from user-input points
def drawCurve(p,q):
    x1_values = p[:,0]
    y1_values = p[:,1]

    x2_values = q[:,0]
    y2_values = q[:,1]

    imageCurve = A.copy()

    curve1 = np.column_stack((np.array(x1_values, dtype=np.int32), np.array(y1_values, dtype=np.int32)))
    curve2 = np.column_stack((np.array(x2_values, dtype=np.int32), np.array(y2_values, dtype=np.int32)))

    cv2.polylines(imageCurve, [curve1], False, (0, 50, 255), lineType=cv2.LINE_AA)
    cv2.polylines(imageCurve, [curve2], False, (255,0,0), lineType=cv2.LINE_AA)

    cv2.imshow("imageCurve", imageCurve)
    cv2.imwrite(contour_file,imageCurve)

    
def _length(vector):
  return math.sqrt(vector[0] ** 2 + vector[1] ** 2)

def _distance(point1, point2):
  return _length((point1[0] - point2[0],point1[1] - point2[1]))

def getCoords(value, minimum, maximum):
  return max(min(value,maximum),minimum)

# Image warping function
def warpImage(points):
  image = Image.open(image_file)
  result = img = Image.new("RGB",image.size,"black")

  image_pixels = image.load()
  result_pixels = result.load()
  print(points)
  for y in range(image.size[1]):
    for x in range(image.size[0]):

      offset = [0,0]

      for point in points:
        point_position = (point[2],point[3]) # Target point

        distance = point[2]-point[0] #Dx
        direction = point[3]-point[1] #Dy
        shift_vector = (distance,direction)

        #determing the amount of movement for neighboring pixel depend on the distance to the control point
        weight = 1.0 / (3 * (_distance((x,y),point_position) / _length(shift_vector)) ** 4 + 5)
        offset[0] -= weight * shift_vector[0]
        offset[1] -= weight * shift_vector[1]

      coords = (getCoords(x + int(offset[0]),0,image.size[0] - 1),getCoords(y + int(offset[1]),0,image.size[1] - 1))

      result_pixels[x,y] = image_pixels[coords[0],coords[1]]

  print("Finish")
  result.show()
  result.save(output_file,'png')

# Callback function for track bar    
def callback(x):
    print(x)

## driver function
if __name__=="__main__":
    # deformation internal parameters
    alpha = 2.1

    # read image
    A = cv2.imread(image_file)

    #convert image to binary image
    gray = cv2.cvtColor(A, cv2.COLOR_BGR2GRAY)

    #Apply Gaussian Blur to filtering out noise
    blur = cv2.GaussianBlur(gray, (3,3), 0)

    # Fine tuning threshold value for Canny edge detection algorithm
    #edged = cv2.Canny(gray, threshold1, threshold2)
    cv2.namedWindow('image') # make a window with name 'image'
    cv2.createTrackbar('L', 'image', 0, 255, callback) #lower threshold trackbar for window 'image
    cv2.createTrackbar('U', 'image', 0, 255, callback) #upper threshold trackbar for window 'image

    print('Press enter or esc when edge detection is completed.')
    while(1):
        k = cv2.waitKey(1) & 0xFF
        
        if k == 27 or k == 13: #escape key or enter pressed
            break
        l = cv2.getTrackbarPos('L', 'image')
        u = cv2.getTrackbarPos('U', 'image')

        edged = cv2.Canny(gray,l,u)
        cv2.imshow('image',edged)
    cv2.waitKey(0)

    # Finding Contours
    contours, hierarchy = cv2.findContours(edged, 
        cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
  
    cv2.imshow('Canny Edges After Contouring', edged)
    cv2.imwrite(edge_file,edged)
    print("Number of Contours found = " + str(len(contours)))
    cv2.waitKey(0)

    # Draw all contours, -1 signifies drawing all contours
    cv2.drawContours(A, contours, -1, (0, 255, 0), 2)

    #cv2.imshow('thresh', thresh)
    cv2.imshow('image', A)

    # setting mouse hadler for the image and calling the click_event() function
    cv2.setMouseCallback('image', click_event)
  
    # wait for a key to be pressed to exit
    cv2.waitKey(0)
  
    # close the window
    cv2.destroyAllWindows()