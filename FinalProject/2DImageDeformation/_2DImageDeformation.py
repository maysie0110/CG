

import numpy as np
import cv2
import scipy.interpolate
from scipy.interpolate import make_interp_spline, BSpline, CubicSpline, UnivariateSpline,interp1d
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
            ###splineCurve(p)
            drawCurve(p,q)
            points = []
            for i in range(count1):
                point = (x1_values[i],y1_values[i],x2_values[i],y2_values[i])
                points.append(point)

            count1 = 0
            count2 = 0

            #apply warping function to the user-specified feature curve
            warp(points)

#def splineCurve(x,y):
def splineCurve(points):
    ##print(points)
    ##points = np.array(points)

    print(points)
    ind=np.argsort(points[:,0])
    a = points[ind]
    print(ind)
    print(a)
    x = a[:,0]
    y = a[:,1]
    ## calculate natural cubic spline polynomials
    #cs = CubicSpline(x,y,bc_type='natural')
    ##f = interp1d(x, y)
    ##f2 = interp1d(x, y, kind='cubic')
   

    ## show values of interpolation function at x=1.25
    ##print('S(1.25) = ', cs(1.25))
    ###x_new = np.linspace(0, 2, 100)
    #x_new = np.linspace(min(x),max(x),100)
    #y_new = cs(x_new)

    ## get x and y vectors
    #x = points[:,0]
    #y = points[:,1]

    ## calculate polynomial
    #z = np.polyfit(x, y, 3)
    #f = np.poly1d(z)

    ## calculate new x's and y's
    #x_new = np.linspace(x[0], x[-1], 50)
    #y_new = f(x_new)

    s = UnivariateSpline(x, y, s=0)
    x_new = np.linspace(min(x),max(x),100)
    y_new = s(x_new)
    drawCurve(x_new,y_new)

# draw curve from user-input points
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

    
def vector_length(vector):
  return math.sqrt(vector[0] ** 2 + vector[1] ** 2)

def points_distance(point1, point2):
  return vector_length((point1[0] - point2[0],point1[1] - point2[1]))

def clamp(value, minimum, maximum):
  return max(min(value,maximum),minimum)

# warping function
def warp(points):
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

        helper = 1.0 / (3 * (points_distance((x,y),point_position) / vector_length(shift_vector)) ** 4 + 5)

        offset[0] -= helper * shift_vector[0]
        offset[1] -= helper * shift_vector[1]

      coords = (clamp(x + int(offset[0]),0,image.size[0] - 1),clamp(y + int(offset[1]),0,image.size[1] - 1))

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