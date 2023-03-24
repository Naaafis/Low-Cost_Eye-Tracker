This week the effort was to attempt running the available pretrrained neural network that was meant to solve our eye tracking. The github repository we used for this task can be found here: https://github.com/hugochan/Eye-Tracker

We ran into some immediate issues as the main python file that handles user interaction with the saved model was written in python 2. I proceeded to upgrade all the print statements to be compatible with my python installation. This meant changing them to follow the ```print(f'....{variable}')``` standard. 

I also had some initial trouble running the following command:

```
python itracker_adv.py -i input_data -lm saved_model
```

It wasnt clarified in the instructions that the ```input_data and saved_model``` arguements are meant to be full paths to where the model was saved and where the .npz datafile downloaded from: http://hugochan.net/download/eye_tracker_train_and_val.npz is saved. 

Despite the efforts of even converting tensorflow calls to the python2 version using the following:

tf.compat.v1.Session(), there were just too many errors after errors to go through for the chance that this model works with our data. Not to mention the face_grid issue mentioned in our proposal. The final error I saw is shown in tf_error.txt. This Does not seem worth resolving, and we will not proceed with a cv-only solution from here on out. 

The file that I have worked on is copied to this directory as, itracker_adv.py. I do not plan to use this file as a part of the final product. 