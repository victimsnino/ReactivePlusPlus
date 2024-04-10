add_executable(linesfrombytes
linesfrombytes.cpp
)

target_link_libraries(linesfrombytes PRIVATE RPP::rpp)
set_target_properties(linesfrombytes PROPERTIES FOLDER Examples/rpp)
