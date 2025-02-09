# clownmdemu-cocoa-test
A very minimal (and broken) Mac native frontend implementation for [Clownacy's ClownMDEmu Mega Drive emulator](https://github.com/Clownacy/clownmdemu-core).

This is just a hobby project that I might work on from time to time, so don't expect any serious developments.

## Building
1. Clone the repo:
	```bash
	git clone https://github.com/WinFutureX/clownmdemu-cocoa-test.git
	```
2. Initialise submodules as the emulator core is built around them (very important!)
	```bash
	cd clownmdemu-cocoa-test
	git submodule update --init --recursive
	```
3. Build.
	```bash
	make
	```
	If you've already built the app and want to update and build from scratch again, do:
	```bash
	git pull # update repo
	git submodule update --init --recursive # update submodules
	make clean && make
	```
## Licence
This app itself is licensed under GPLv3 (see `LICENSE.txt`), while the emulator core and associated libraries are licensed under AGPLv3 (see `clownmdemu-frontend-common/LICENCE.txt`).
