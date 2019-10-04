# Trurt

Trurt is a program that detects faces in video streams and mirrors them.

![An example](/images/trurt.jpg)


## What Is This I Don't Even

[Unitinu](https://knowyourmeme.com/memes/unitinu-uniti-u) is an old meme that involves mirroring photos for comedic effect. These could be made using Photoshop or even with a dedicated tool like [this one](https://github.com/panicsteve/unitinu) for OS X or [this one](https://github.com/niki1337/UNITINU-Image-Tool) in Javascript. However, all of these require you to choose the mirroring point manually for maximum lulz. But why go through that effort when you could outsource the selection to that master of comedy: an unthinking, emotionless automaton.

Trurt uses Mateo Hrastnik's [face tracker](https://github.com/hrastnik/face_detect_n_track) to detect and track a face and the facial landmark model from [dlib](https://github.com/davisking/dlib) to automatically select the mirroring point (at the top of the nasal bridge). Since this is relatively simple process, it can be applied to video streams on a frame-by-frame basis, and indeed that is all that this program does.

Face detection based on Haar wavelets is known to be occasionally hokey. However, this can be considered a feature rather than a bug, since poor face identification can sometimes make the result funnier.

![Another example](/images/trurt2.jpg)


## Build Instructions

You must first download the dlib face landmark model, which is a bit uncomfortably large (96M) to store in a git repository, and copy it to (or symlink it as) `data/landmarks68.dat`.

You must have OpenCV installed. You also need cmake and the usual build-essential packages.

```
git clone https://github.com/larilampen/trurt.git
wget http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2
bzip -d shape_predictor_68_face_landmarks.dat.bz2
mv shape_predictor_68_face_landmarks.dat trurt/data/landmarks68.dat
cd trurt
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make
```

If you use an IDE like [Qt Creator](https://www.qt.io/development-tools), the latter half of the instructions above is pretty much reduced to a button press.


## Usage

There are numerous command line parameters, which you can list by running `./Trurt --help`. You can specify an input file with `-i <file>` (which should be a video file in a format that OpenCV can read); if no file is specified, the program will try to open camera device 0 instead. To save the output into video files, use `-s`. If the default mp4 encoding isn't working, you can use the `-j` option to fall back to OpenCV's built-in Motion JPEG encoder.

Hotkeys: press `l` to toggle display of facial landmarks, `m` to toggle display of the mirroring line, and `p` to pause the program until a keypress.


## Authors

Code: Lari Lampen.

Significant portions of code appropriated from work by [Mateo Hrastnik](https://github.com/hrastnik).


## License

MIT License.

*Exclusion:* The directory `data` contains the trained Haar wavelet model for face recognition from [OpenCV](https://github.com/opencv/opencv). It is included only for convenience. The file's original license continues to apply.
