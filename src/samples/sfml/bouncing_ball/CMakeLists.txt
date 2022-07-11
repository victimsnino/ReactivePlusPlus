add_executable(bouncing_ball
    main.cpp
)

target_link_libraries(bouncing_ball PRIVATE rpp SFML::Graphics)
set_target_properties(bouncing_ball PROPERTIES FOLDER Samples/SFML)

add_custom_command(TARGET bouncing_ball POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:bouncing_ball> $<TARGET_FILE_DIR:bouncing_ball>
  COMMAND_EXPAND_LISTS
)