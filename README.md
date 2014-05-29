Active3D Desktop Pipeline
===

Built with OpenCV and GLUT.

Image -> Segment foreground/background -> 3D reconstruction -> Stereo view -> Active 3D

Takes a 2D image and performs a user assisted GrabCut segmantation. The two segmented images (foreground and background) are reconstructed in 3D and rendered from different perspectives to generate a stereo pair for side by side 3D viewing.

Usage
===

```
active3d [imagefile]
```

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
