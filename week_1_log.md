The difficulties presented by this weeks task came from installing Dlib itself. While this library is known to perform well on face detection, it does not 
seem like it would be possible to mount this library onto an ESP32 due to the sheer amount of prequisites. I was directed to install boost, cmake, scikit-
image, scipy and eventually had to settle for a solution using conda-forge to install this library. While turning my computer into a server, or deploying 
the requirements into an actual AWS EC2 server would allow for the implementation of these prerequisites, it would be nearly impossible to do the same on a 
resource-constrained microcontroller. 

Great learning experience installing the libraries none the less. I was forced to look up every library's purpose after analyzing imports of pre-existing 
eye-tracking projects. This allowed me to not only complete the "resources" section of our proposal, but also understand the various use cases behind 
aforementioned libraries. 

Additionally, I got refreshed on how to create a python virtual environment using 

```
conda create --name eye_tracker python=3
```

And to activate the environment using 

```
conda activate eye_tracker
```

The environment could later be deactivated to go back to base python libraries installed on the OS by using:

```
conda deactivate eye_tracker
```

I also got reminded to check each library installation using 

```
pip show [library name]
```

Of course, in order for that command to work, the library had to be installed using 

```
pip install [library name]
```

The `dlib` library was installed using conda-forge, and its installation was verified using 

```
conda list dlib
```

Thankfully, my computer was already equipped with Python3, Pip, Homebrew and Anaconda. The aliases for these package managers were also properly setup for 
my liking. This made the installation process much smoother. 

The complete list of commands to install all necessary libraries:

```
pip install --upgrade pip 
pip install numpy
pip install scipy
pip install tensorflow
pip install matplotlib
pip install opencv_python
pip install Flask
pip install pytest-timeit
brew install cmake
pip install cmake
brew install boost
brew install boost-python3
pip install scikit-image
conda install -c conda-forge dlib
```

Lastly, the requirements.txt file was compiled using the command

```
pip freeze > requirements.txt
```

All the installed libraries for the specific virtual environment that I created for this project are the only ones that is present in that file.

One last note: I downloaded the dataset provided by citation [7] in the proposal. It is large at 1.99Gbs so I will not bother to upload the data to Github. 
