import cv2
import numpy as np
import base64
from PIL import Image
import io

# Load haarcascades for face and eyes
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
eyes_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')

# function to decode base64 encoded image 
# def decode_image(base64_image):
#     # the base64 images provided appear to have an additional header
#     base64_image = base64_image.split(",")[1]  # split on comma and take the second part
#     imgdata = base64.b64decode(base64_image)
#     with Image.open(io.BytesIO(imgdata)) as image:
#         np_image = np.array(image)
#     # Ensure resources are released
#     del image
#     return cv2.cvtColor(np_image, cv2.COLOR_BGR2RGB)

def decode_image(image_data):
    # Convert bytes to string
    image_data = image_data.decode('utf-8')
    
    # Convert the string to bytes
    image_data = bytes.fromhex(image_data)
    
    # Read the image data using a BytesIO object
    with Image.open(io.BytesIO(image_data)) as image:
        np_image = np.array(image)
    # Ensure resources are released
    del image
    # Convert to RGB
    return cv2.cvtColor(np_image, cv2.COLOR_BGR2RGB)

# Function to detect face
def detect_face(image):
    faces = face_cascade.detectMultiScale(image, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    for (x,y,w,h) in faces:
        face = image[y:y+h, x:x+w]
    return face

# function to detect eyes and cut out the eyebrows
def detect_eyes(face):
    eyes = eyes_cascade.detectMultiScale(face) #, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    detected_eyes = []
    for (ex,ey,ew,eh) in eyes:
       # cut out the top 25% of the eye rectangle to remove the eyebrow
       ey = int(ey + 0.25*eh)
       eh = int(eh - 0.25*eh)
       eye = face[ey:ey+eh, ex:ex+ew]
       detected_eyes.append(eye)
    return detected_eyes

# Function to process eyes images
def process_eyes(eye):
    # Convert to grayscale
    gray_eye = cv2.cvtColor(eye, cv2.COLOR_BGR2GRAY)
    
    # Threshold the image
    _, thresh = cv2.threshold(gray_eye, 60, 255, cv2.THRESH_BINARY)
    
    # Erode and dilate to remove some noise
    kernel = np.ones((2,2),np.uint8)
    thresh = cv2.erode(thresh, kernel, iterations = 2)
    thresh = cv2.dilate(thresh, kernel, iterations = 4)
    
    # Median blur to smooth image
    smooth_eye = cv2.medianBlur(thresh, 5)
    
    return smooth_eye

# Function to detect pupil using Blob detector
def detect_pupil(smooth_eye):
    # Setup SimpleBlobDetector parameters
    params = cv2.SimpleBlobDetector_Params()
    params.filterByArea = True
    params.minArea = 10

    # Create a detector with the parameters
    detector = cv2.SimpleBlobDetector_create(params)

    keypoints = detector.detect(smooth_eye)

    # Get the coordinates of the blobs (pupils)
    points = [(int(k.pt[0]), int(k.pt[1])) for k in keypoints]

    return points

# Function to determine pupil direction
def pupil_direction(pupil, eye):
    mid_point = int(eye.shape[1] / 2)

    if abs(pupil[0] - mid_point) < 5:
        direction = 'center'
    elif pupil[0] - mid_point < 0:
        direction = 'left'
    else:
        direction = 'right'

    return direction

def eye_tracker(image):
    # Create a copy of the image to avoid modifying the original image
    image_copy = image.copy()

    # Detect face
    face_img = detect_face(image_copy)

    # Detect eyes
    eyes = detect_eyes(face_img)

    # Initialize a list to hold the pupil directions
    pupil_directions = []

    # Process eyes and detect pupils
    for i, eye in enumerate(eyes):
        processed_eye = process_eyes(eye)
        pupils = detect_pupil(processed_eye)

        # If a pupil is detected
        if pupils is not None and len(pupils) == 1:
            pupil = pupils[0]
            # Determine pupil direction
            direction = pupil_direction(pupil, eye)
            print(f"Eye {i+1}: Pupil is looking {direction}")

            # Add the pupil direction to the list
            pupil_directions.append(direction)

        else:
            print(f"Pupil in Eye {i+1} not detected.")

    # Return the list of pupil directions
    return pupil_directions

from flask import Flask, request, jsonify
app = Flask(__name__)


@app.route('/', methods=['POST'])
# def eye_tracker_api():
#     # Get the image data from the request
#     image_data = request.files['image'].read()
    
#     # Convert bytes to base64 string
#     image_data = base64.b64encode(image_data).decode('utf-8')
        
#     # decode the image data
#     image_data = decode_image(image_data)
# def eye_tracker_api():
#     # Get the image data from the request
#     image_data = request.data
    
#     # decode the image data
#     image_data = decode_image(image_data)
    
#     # grab the face from the image
#     face_img = detect_face(image_data)

#     # Process the image data using the eye_tracker function
#     pupil_directions = eye_tracker(face_img)

#     # Return the pupil directions as a JSON response
#     return {'pupil_directions': pupil_directions}
def eye_tracker_api():
    # Get the image data from the request
    image_data = request.data

    # may need to update this to split on the first comma
    # image_data = image_data.split(b",")[1]

    # decode the image data
    image = decode_image(image_data)
    
    # grab the face from the image
    face_img = detect_face(image)

    # Process the image data using the eye_tracker function
    pupil_directions = eye_tracker(face_img)

    # Return the pupil directions as a JSON response
    # return jsonify({'pupil_directions': pupil_directions[0]})
    return pupil_directions[0]