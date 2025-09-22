#!/bin/bash

# OpenCV를 사용하여 C++ 파일을 컴파일하는 스크립트

echo "Compiling stitch_images.cpp..."

g++ stitch_images.cpp -o stitch_images \
    -I/usr/include/opencv4 \
    -L/usr/lib/x86_64-linux-gnu \
    -lopencv_core \
    -lopencv_imgcodecs \
    -lopencv_stitching \
    -lopencv_imgproc

# 컴파일 성공 여부 확인
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "You can now run the program with: ./stitch_images <image1.jpg> <image2.jpg> ..."
else
    echo "Compilation failed. Please check the error messages above."
fi
