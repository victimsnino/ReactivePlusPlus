set(TARGET snake)

add_executable(${TARGET}
    snake.cpp
)

target_link_libraries(${TARGET} PRIVATE rpp SFML::Graphics)
set_target_properties(${TARGET} PROPERTIES FOLDER Samples/SFML)

add_custom_command(TARGET ${TARGET} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
  COMMAND_EXPAND_LISTS
)