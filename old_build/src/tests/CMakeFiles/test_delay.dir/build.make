# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/victi/ReactivePlusPlus

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/victi/ReactivePlusPlus/build

# Include any dependencies generated for this target.
include src/tests/CMakeFiles/test_delay.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/tests/CMakeFiles/test_delay.dir/compiler_depend.make

# Include the progress variables for this target.
include src/tests/CMakeFiles/test_delay.dir/progress.make

# Include the compile flags for this target's objects.
include src/tests/CMakeFiles/test_delay.dir/flags.make

src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o: src/tests/CMakeFiles/test_delay.dir/flags.make
src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o: ../src/tests/rpp/test_delay.cpp
src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o: src/tests/CMakeFiles/test_delay.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/victi/ReactivePlusPlus/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o"
	cd /home/victi/ReactivePlusPlus/build/src/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o -MF CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o.d -o CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o -c /home/victi/ReactivePlusPlus/src/tests/rpp/test_delay.cpp

src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_delay.dir/rpp/test_delay.cpp.i"
	cd /home/victi/ReactivePlusPlus/build/src/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/victi/ReactivePlusPlus/src/tests/rpp/test_delay.cpp > CMakeFiles/test_delay.dir/rpp/test_delay.cpp.i

src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_delay.dir/rpp/test_delay.cpp.s"
	cd /home/victi/ReactivePlusPlus/build/src/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/victi/ReactivePlusPlus/src/tests/rpp/test_delay.cpp -o CMakeFiles/test_delay.dir/rpp/test_delay.cpp.s

# Object files for target test_delay
test_delay_OBJECTS = \
"CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o"

# External object files for target test_delay
test_delay_EXTERNAL_OBJECTS =

bin/test_delay: src/tests/CMakeFiles/test_delay.dir/rpp/test_delay.cpp.o
bin/test_delay: src/tests/CMakeFiles/test_delay.dir/build.make
bin/test_delay: bin/libsnitch.a
bin/test_delay: src/tests/CMakeFiles/test_delay.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/victi/ReactivePlusPlus/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/test_delay"
	cd /home/victi/ReactivePlusPlus/build/src/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_delay.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/tests/CMakeFiles/test_delay.dir/build: bin/test_delay
.PHONY : src/tests/CMakeFiles/test_delay.dir/build

src/tests/CMakeFiles/test_delay.dir/clean:
	cd /home/victi/ReactivePlusPlus/build/src/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_delay.dir/cmake_clean.cmake
.PHONY : src/tests/CMakeFiles/test_delay.dir/clean

src/tests/CMakeFiles/test_delay.dir/depend:
	cd /home/victi/ReactivePlusPlus/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/victi/ReactivePlusPlus /home/victi/ReactivePlusPlus/src/tests /home/victi/ReactivePlusPlus/build /home/victi/ReactivePlusPlus/build/src/tests /home/victi/ReactivePlusPlus/build/src/tests/CMakeFiles/test_delay.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/tests/CMakeFiles/test_delay.dir/depend

