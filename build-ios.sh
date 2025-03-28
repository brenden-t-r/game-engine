#!/bin/bash

rm -r build
mkdir build
pushd build
cmake -G Xcode -DBACKEND_METAL=ON -DPLATFORM_IOS=ON ..
cmake --build . --config Debug
popd build

if [ "$1" == "-run" ]; then
  echo "Running.."
  xcrun simctl install booted build/Debug-iphonesimulator/GameEngine.app
  xcrun simctl launch booted com.mycompany.mygame
else
  echo "Opening in Xcode.."
  open GameEngine.xcodeproj
fi