## This file is also being used to generate our build flags document at https://intelrealsense.github.io/librealsense/build-flags-docs/build-flags.html
## Formatting notes for this file:
## Options are listed as: <name> | <description> [comment] | <value>
## regular comments should be ABOVE their relevent option
## use double # for comments that should not show in the options doc

option(ENABLE_CCACHE "Build with ccache." ON)
option(BUILD_WITH_CUDA "Enable CUDA" OFF)
option(BUILD_GLSL_EXTENSIONS "Build GLSL extensions API" ON)
option(BUILD_WITH_OPENMP "Use OpenMP" OFF)
option(BUILD_EASYLOGGINGPP "Build EasyLogging++ as a part of the build" ON)
option(BUILD_WITH_STATIC_CRT "Build with static link CRT" ON)
option(HWM_OVER_XU "Send HWM commands over UVC XU control" ON)
option(COM_MULTITHREADED "Set OFF to initialize COM library with COINIT_APARTMENTTHREADED (Windows only)" ON)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_UNIT_TESTS "Build LibCI unit tests. If enabled, additional test data may be downloaded" OFF)
option(BUILD_EXAMPLES "Build examples (not including graphical examples -- see BUILD_GRAPHICAL_EXAMPLES)" ON)
option(BUILD_GRAPHICAL_EXAMPLES "Build graphical examples (Viewer & DQT) -- Implies BUILD_GLSL_EXTENSIONS" ON)
option(BUILD_CV_EXAMPLES "Build OpenCV examples" OFF)
option(BUILD_DLIB_EXAMPLES "Build DLIB examples - requires DLIB_DIR" OFF)
option(BUILD_PCL_EXAMPLES "Build PCL examples" OFF)
option(BUILD_TOOLS "Build tools (fw-updater, etc.) that are not examples" ON)
option(ENFORCE_METADATA "Require WinSDK with Metadata support during compilation. Windows OS Only" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
option(BUILD_LEGACY_PYBACKEND "Build deprecated Python backend bindings" OFF)
option(BUILD_PYTHON_DOCS "Build Documentation for Python bindings" OFF)
option(BUILD_CSHARP_BINDINGS "Build C# bindings" OFF)
option(BUILD_MATLAB_BINDINGS "Build Matlab bindings" OFF)
option(BUILD_UNITY_BINDINGS "Copy the unity project to the build folder with the required dependencies" OFF)
option(BUILD_OPENVINO_EXAMPLES "Build Intel OpenVINO Toolkit examples - requires INTEL_OPENVINO_DIR" OFF)
option(BUILD_OPEN3D_EXAMPLES "Build Open3D examples" OFF)
option(BUILD_OPENNI2_BINDINGS "Build OpenNI bindings" OFF)
option(IMPORT_DEPTH_CAM_FW "Download the latest firmware for the depth cameras" ON)
option(BUILD_CV_KINFU_EXAMPLE "Build OpenCV KinectFusion example" OFF)
option(FORCE_RSUSB_BACKEND "Use RS USB backend, mandatory for Win7/MacOS/Android, optional for Linux" OFF)
option(FORCE_LIBUVC "Explicitly turn-on libuvc backend - deprecated, use FORCE_RSUSB_BACKEND instead" OFF)
option(FORCE_WINUSB_UVC "Explicitly turn-on winusb_uvc (for win7) backend - deprecated, use FORCE_RSUSB_BACKEND instead" OFF)
option(ANDROID_USB_HOST_UVC "Build UVC backend for Android - deprecated, use FORCE_RSUSB_BACKEND instead" OFF)
# This feature requires OpenSSL installation on Linux/OSX, OSX normally does not come with OpenSSL integrated(Thats why default is OFF on OSX)
if (NOT APPLE)
    option(CHECK_FOR_UPDATES "Checks for versions updates" ON) 
else()
    option(CHECK_FOR_UPDATES "Checks for versions updates" OFF) 
endif()
option(BUILD_WITH_CPU_EXTENSIONS "Enable compiler optimizations using CPU extensions (such as AVX)" ON)
set(UNIT_TESTS_ARGS "" CACHE STRING "Command-line arguments to pass to unit-tests-config.py, e.g. '-t <tag> -r <regex>'")
#Performance improvement with Ubuntu 18/20
if(UNIX AND (NOT ANDROID_NDK_TOOLCHAIN_INCLUDED))
    option(ENABLE_EASYLOGGINGPP_ASYNC "Switch Logger to Asynchronous Mode (set OFF for Synchronous Mode)"  ON)
else()
    option(ENABLE_EASYLOGGINGPP_ASYNC "Switch Logger to Asynchronous Mode (set OFF for Synchronous Mode)" OFF)
endif()
option(BUILD_PC_STITCHING "Build pointcloud-stitching example" OFF)
option(BUILD_WITH_DDS "Access camera devices through DDS topics (requires CMake 3.16.3)" OFF)
option(BUILD_RS2_ALL "Build realsense2-all static bundle containing all realsense libraries (with BUILD_SHARED_LIBS=OFF)" ON)
option(ENABLE_SECURITY_FLAGS "Enable additional compiler security flags to enhance the build's security" OFF)
option(USE_EXTERNAL_LZ4 "Use externally build LZ4 library instead of building and using the in this repo provided version" OFF)
option(BUILD_ASAN "Enable AddressSanitizer" OFF)
mark_as_advanced(BUILD_ASAN)
