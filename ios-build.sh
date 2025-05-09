#!/bin/bash

rm -r build
mkdir build
pushd build
cmake -G Xcode -DBACKEND_METAL=ON -DPLATFORM_IOS=ON -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk ..
echo "Opening in Xcode.."
open GameProject.xcodeproj
popd build
