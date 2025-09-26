.PHONY: game
game:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build && cmake --build . --config Release

.PHONY: xcode
xcode:
	mkdir -p build
	cd build && cmake -GXcode ..
	open build/*.xcodeproj

.PHONY: mac_dist
mac_dist: clean game
	codesign --entitlements mac/Glypha.entitlements -fs "Developer ID Application" "build/Glypha III.app"
	cd build && zip -r "Glypha III.zip" "Glypha III.app"

.PHONY: qt
qt:
	mkdir -p build
	cd build && cmake -DHAVE_QT=true -DCMAKE_BUILD_TYPE=Release ..
	cmake --build build --config Release

.PHONY: qt_xcode
qt_xcode:
	mkdir -p build
	cd build && cmake -GXcode -DHAVE_QT=true ..
	open build/*.xcodeproj

.PHONY: clean
clean:
	rm -rf build*
	rm -rf webbuild/CMakeFiles
	rm -f webbuild/CMakeCache.txt
	rm -f webbuild/Makefile
	rm -f webbuild/cmake_install.cmake
	rm -f webbuild/glypha_iii*
	rm -rf gl4es/build
	rm -rf android/app/.cxx
	rm -rf android/app/build

gl4es/include/GL/gl.h:
	git clone https://github.com/ptitSeb/gl4es.git gl4es

gl4es/lib/libGL.a: gl4es/include/GL/gl.h
	mkdir -p gl4es/build
	cd gl4es/build && emcmake cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNOX11=ON -DNOEGL=ON -DSTATICLIB=ON
	cd gl4es/build && make

mobbuild/icon.png:
	cp ./webbuild/icon.png mobbuild/icon.png

mobbuild/index.html:
	cp ./webbuild/index.html mobbuild/index.html

android/app/src/main/cpp/game:
	rm -f android/app/src/main/cpp/game
	ln -s ../../../../../game android/app/src/main/cpp/game

android/app/src/main/cpp/gl4es: gl4es/include/GL/gl.h
	rm -f android/app/src/main/cpp/gl4es
	ln -s ../../../../../gl4es android/app/src/main/cpp/gl4es
	
android/app/src/main/cpp/resources: game
	rm -f android/app/src/main/cpp/resources
	ln -s ../../../../../build/resources android/app/src/main/cpp/resources

.PHONY: web
web: game gl4es/lib/libGL.a
	cd webbuild && emcmake cmake -DCMAKE_BUILD_TYPE=Release .
	cd webbuild && cmake --build . --config Release

.PHONY: webs
webs: web
	cd webbuild && python3 -m http.server

.PHONY: mobile
mobile: game gl4es/lib/libGL.a mobbuild/icon.png mobbuild/index.html
	cd mobbuild && emcmake cmake -DCMAKE_BUILD_TYPE=Release .
	cd mobbuild && cmake --build . --config Release

.PHONY: mobiles
mobiles: mobile
	cd mobbuild && python3 -m http.server

.PHONY: publish
publish: mobile web
	mkdir -p glypha-iii
	cp mobbuild/glypha_mobile.js glypha-iii
	cp mobbuild/glypha_mobile.wasm glypha-iii
	cp mobbuild/mobile.html glypha-iii
	cp mobbuild/mobilegame.html glypha-iii
	cp webbuild/icon.png glypha-iii
	cp webbuild/glypha_iii.js glypha-iii
	cp webbuild/glypha_iii.wasm glypha-iii
	cp webbuild/game.html glypha-iii
	cp webbuild/index.html glypha-iii
	cp webbuild/controller.min.js glypha-iii
	zip -vr glypha-iii.zip glypha-iii/ -x "*.DS_Store"

.PHONY: android
android: android/app/src/main/cpp/game android/app/src/main/cpp/gl4es android/app/src/main/cpp/resources
	cd android && ./gradlew build
