# Makefile for project compilation

.PHONY: all clean help macos windows

# Default target
all: macos windows

# Path to Max SDK to adapt
C74_SDK ?= /Users/letz/Developpements/faust/max-sdk/source

# Build for macOS
macos:
	@echo "Building for macOS..."
	@cd source && mkdir -p build_macos && cd build_macos && \
	cmake .. -DC74_SDK=$(C74_SDK) -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 && \
	make -j$(shell sysctl -n hw.ncpu)

# Cross-compile for Windows using MinGW
windows:
	@echo "Cross-compiling for Windows..."
	@cd source && mkdir -p build_windows && cd build_windows && \
	cmake .. -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ -DC74_SDK=$(C74_SDK) && \
	make -j$(shell sysctl -n hw.ncpu) 

# Clean build directories
clean:
	@echo "Cleaning build directories..."
	@rm -rf source/build_macos source/build_windows
	
# Format code
format:
	@echo "Formatting source code..."
	@find source -iname '*.c' -execdir clang-format -i -style=file {} \;
	@find source -iname '*.h' -execdir clang-format -i -style=file {} \;

# Help section
help:
	@echo "Available targets:"
	@echo "  all       - Build for macOS and Windows"
	@echo "  macos     - Compile for macOS"
	@echo "  windows   - Cross-compile for Windows using MinGW"
	@echo "  clean     - Remove build directories"
	@echo "  format    - Format source code using clang-format"
	@echo "  help      - Show this help message"
