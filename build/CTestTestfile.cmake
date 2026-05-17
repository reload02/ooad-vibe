# CMake generated Testfile for 
# Source directory: C:/Users/re-ro/Desktop/ooad-vibe
# Build directory: C:/Users/re-ro/Desktop/ooad-vibe/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("C:/Users/re-ro/Desktop/ooad-vibe/build/rvc_tests[1]_include.cmake")
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[SimulatorCliDefaultRuns]=] "C:/Users/re-ro/Desktop/ooad-vibe/build/Debug/rvc_simulator.exe" "--ticks" "5" "--quiet-map")
  set_tests_properties([=[SimulatorCliDefaultRuns]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;57;add_test;C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[SimulatorCliDefaultRuns]=] "C:/Users/re-ro/Desktop/ooad-vibe/build/Release/rvc_simulator.exe" "--ticks" "5" "--quiet-map")
  set_tests_properties([=[SimulatorCliDefaultRuns]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;57;add_test;C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[SimulatorCliDefaultRuns]=] "C:/Users/re-ro/Desktop/ooad-vibe/build/MinSizeRel/rvc_simulator.exe" "--ticks" "5" "--quiet-map")
  set_tests_properties([=[SimulatorCliDefaultRuns]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;57;add_test;C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[SimulatorCliDefaultRuns]=] "C:/Users/re-ro/Desktop/ooad-vibe/build/RelWithDebInfo/rvc_simulator.exe" "--ticks" "5" "--quiet-map")
  set_tests_properties([=[SimulatorCliDefaultRuns]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;57;add_test;C:/Users/re-ro/Desktop/ooad-vibe/CMakeLists.txt;0;")
else()
  add_test([=[SimulatorCliDefaultRuns]=] NOT_AVAILABLE)
endif()
subdirs("_deps/googletest-build")
