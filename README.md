Active3D Desktop Pipeline
===

Built with OpenCV and GLUT. Takes an input image and creates a stereogram after reconstructing the foreground in 3D.

Image -> Segment foreground/background -> Depth function -> 3D reconstruction -> Stereo pair

Usage
===

```
active3d [imagefile]
```

Example: `./active3d img/coffee.jpg`

1. Draw a rectangle around the foreground of the image to begin foreground segmentation.
2. The segmentation result is now displayed. Here you can make any necessary refinements. Painting over the image with the mouse will mark painted areas as background regions. If you need to add a region back to the image, press 'M' to switch into foreground painting mode.
3. Press the 'Enter' key to start 3D reconstruction.
4. A stereo pair is generated and displayed in a GLUT window. You can use the arrow keys to rotate the camera (warning, slow!).
5. Press 'Esc' to quit at any time.

Some sample image files are proved under `img/`

Dependencies
===

Requires GLUT and OpenGL.

```
sudo apt-get install freeglut3-dev
```

Requires OpenCV2 to be installed in a system wide path on Linux. For OS X, the compile script assumes a Macports installation of OpenCV. The following libraries are used:

- opencv_core
- opencv_imgproc
- opencv_photo
- openvc_highgui

Compilation
===
Debian: `./compile_active3d_ubuntu.sh`

Mac OS X: `./compile_active3d_macports.sh`
