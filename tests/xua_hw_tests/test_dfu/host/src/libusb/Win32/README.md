# Generating Required Files

This guide outlines the steps to generate necessary files in this folder using a Windows environment.

## Prerequisites

- Windows operating system
- Visual Studio 2022 with Visual Studio tools installed

## Instructions

1. **Install Visual Studio 2022 Tools**: Ensure that Visual Studio 2022 is installed with the necessary tools for C/C++ development.

2. **Download libusb-1.0 Source Code**:
   - Navigate to the [libusb GitHub repository](https://github.com/libusb/libusb) and download the source code for version 1.0.27 as a ZIP file from [this link](https://github.com/libusb/libusb/archive/refs/tags/v1.0.27.zip).

3. **Extract the ZIP Archive**:
   - Unzip the downloaded archive to a convenient location on your machine.

4. **Copy Header File**:
   - Locate the `libusb.h` file in the extracted folder at `libusb-1.0.27\libusb`.
   - Copy `libusb.h` to the folder where you want the generated files to reside.

5. **Open Command Prompt**:
   - Open an "x86 Native Tools Command Prompt for VS 2022". This can be found in the Start Menu under Visual Studio 2022 Tools.

6. **Build libusb**:
   - Navigate to the `msvc` directory within the extracted `libusb-1.0.27` folder.
   - Execute the following command to build libusb:
     ```
     msbuild -p:PlatformToolset=v143,Platform=win32,Configuration=Release libusb.sln
     ```
   - This command compiles the libusb solution using the Visual Studio 2022 (v143) toolset for the Win32 platform in Release configuration.

7. **Copy Generated Library**:
   - After the build completes, locate the generated `libusb-1.0.lib` file in `libusb-1.0.27\build\v143\Win32\Release\lib`.
   - Copy `libusb-1.0.lib` to the folder where you want the generated files to reside.

## Notes

- Ensure that you have administrative rights if you encounter permission issues during these steps.
- For any issues related to Visual Studio tools or the build process, refer to the Visual Studio 2022 documentation or the libusb GitHub repository's issue tracker.