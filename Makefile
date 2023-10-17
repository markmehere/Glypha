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
	rm webbuild/CMakeCache.txt
	rm webbuild/Makefile
	rm webbuild/cmake_install.cmake
	rm webbuild/glypha_iii*
	rm -rf webbuild/gl4es/build

webbuild/gl4es/include/GL/gl.h:
	git clone https://github.com/ptitSeb/gl4es.git webbuild/gl4es

webbuild/gl4es/lib/libGL.a: webbuild/gl4es/include/GL/gl.h
	mkdir -p webbuild/gl4es/build
	cd webbuild/gl4es/build && emcmake cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNOX11=ON -DNOEGL=ON -DSTATICLIB=ON
	cd webbuild/gl4es/build && make

mobbuild/icon.png:
	cp ./webbuild/icon.png mobbuild/icon.png

mobbuild/index.html:
	cp ./webbuild/index.html mobbuild/index.html

.PHONY: web
web: game webbuild/gl4es/lib/libGL.a
	cd webbuild && emcmake cmake -DCMAKE_BUILD_TYPE=Release .
	cd webbuild && cmake --build . --config Release

.PHONY: webs
webs: web
	cd webbuild && python3 -m http.server

.PHONY: mobile
mobile: game webbuild/gl4es/lib/libGL.a mobbuild/icon.png mobbuild/index.html
	cd mobbuild && emcmake cmake -DCMAKE_BUILD_TYPE=Release .
	cd mobbuild && cmake --build . --config Release

.PHONY: mobiles
mobiles: mobile
	cd mobbuild && python3 -m http.server
