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
include src/tests/CMakeFiles/test_last.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/tests/CMakeFiles/test_last.dir/compiler_depend.make

# Include the progress variables for this target.
include src/tests/CMakeFiles/test_last.dir/progress.make

# Include the compile flags for this target's objects.
include src/tests/CMakeFiles/test_last.dir/flags.make

src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.o: src/tests/CMakeFiles/test_last.dir/flags.make
src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.o: ../src/tests/rpp/test_last.cpp
src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.o: src/tests/CMakeFiles/test_last.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/victi/ReactivePlusPlus/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.o"
	cd /home/victi/ReactivePlusPlus/build/src/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.o -MF CMakeFiles/test_last.dir/rpp/test_last.cpp.o.d -o CMakeFiles/test_last.dir/rpp/test_last.cpp.o -c /home/victi/ReactivePlusPlus/src/tests/rpp/test_last.cpp

src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_last.dir/rpp/test_last.cpp.i"
	cd /home/victi/ReactivePlusPlus/build/src/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/victi/ReactivePlusPlus/src/tests/rpp/test_last.cpp > CMakeFiles/test_last.dir/rpp/test_last.cpp.i

src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_last.dir/rpp/test_last.cpp.s"
	cd /home/victi/ReactivePlusPlus/build/src/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/victi/ReactivePlusPlus/src/tests/rpp/test_last.cpp -o CMakeFiles/test_last.dir/rpp/test_last.cpp.s

# Object files for target test_last
test_last_OBJECTS = \
"CMakeFiles/test_last.dir/rpp/test_last.cpp.o"

# External object files for target test_last
test_last_EXTERNAL_OBJECTS =

bin/test_last: src/tests/CMakeFiles/test_last.dir/rpp/test_last.cpp.o
bin/test_last: src/tests/CMakeFiles/test_last.dir/build.make
bin/test_last: bin/libsnitch.a
bin/test_last: src/tests/CMakeFiles/test_last.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/victi/ReactivePlusPlus/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/test_last"
	cd /home/victi/ReactivePlusPlus/build/src/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_last.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/tests/CMakeFiles/test_last.dir/build: bin/test_last
.PHONY : src/tests/CMakeFiles/test_last.dir/build

src/tests/CMakeFiles/test_last.dir/clean:
	cd /home/victi/ReactivePlusPlus/build/src/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_last.dir/cmake_clean.cmake
.PHONY : src/tests/CMakeFiles/test_last.dir/clean

src/tests/CMakeFiles/test_last.dir/depend:
	cd /home/victi/ReactivePlusPlus/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/victi/ReactivePlusPlus /home/victi/ReactivePlusPlus/src/tests /home/victi/ReactivePlusPlus/build /home/victi/ReactivePlusPlus/build/src/tests /home/victi/ReactivePlusPlus/build/src/tests/CMakeFiles/test_last.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/tests/CMakeFiles/test_last.dir/depend

