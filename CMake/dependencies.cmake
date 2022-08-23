# MIT License
# 
# Copyright (c) 2022 Aleksey Loginov
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

find_package(Threads REQUIRED)

# ===================== SFML =======================
if (RPP_BUILD_SFML_CODE)
    find_package(SFML COMPONENTS graphics system window REQUIRED)
endif()

# ==================== RXCPP =======================
if (RPP_AUTOTESTS)
  Include(FetchContent)

  FetchContent_Declare(
    RxCpp
    GIT_REPOSITORY https://github.com/ReactiveX/RxCpp.git
    GIT_TAG        origin/main
  )

  FetchContent_MakeAvailable(RxCpp)
endif()

# ===================== Catch 2 ===================
if (RPP_BUILD_TESTS)
  find_package(Catch2 3 QUIET)

  if(TARGET Catch2::Catch2WithMain)
    message("-- RPP: Catch2 found as package")
  else()
    message("-- RPP: Catch2 not found, fetching from github... Set Catch2_DIR if you have installed Catch2")
    Include(FetchContent)
    
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v3.1.0
    )
    FetchContent_MakeAvailable(Catch2)
  endif()
endif()