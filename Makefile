# vim: set noexpandtab tabstop=4:

help:  ## Display this help
		@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n\nTargets:\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-13s\033[0m %s\n", $$1, $$2 }' $(MAKEFILE_LIST)

menuconfig: ## Open menuconfig
	ninja -C build menuconfig

local_build: ## Build local CMake build
	ninja -C build

local_setup: ## Setup local CMake build (With compile commands)
	cmake -S . -B build -G Ninja \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DYAK_ARCH=x86_64 \
		-DYAK_TOOLCHAIN=clang
