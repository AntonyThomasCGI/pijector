# Pijector

Project gifs on the wall via a raspberry pi connected to a projector


Build with DRM support:
```
mkdir build
cd build
cmake .. \
  -DPLATFORM=DRM \
  -DUSE_EXTERNAL_GLFW=OFF \
  -DGRAPHICS=GLES2
cmake --build .
```