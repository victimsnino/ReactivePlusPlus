set(TARGET snake)

add_executable(${TARGET}
    main.cpp

    snake.cpp
    snake.hpp

    utils.hpp

    canvas.hpp
    canvas.cpp
)

target_link_libraries(${TARGET} PRIVATE rpp SFML::Graphics)
set_target_properties(${TARGET} PROPERTIES FOLDER Samples/SFML)

if (WIN32)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SFML::Graphics> $<TARGET_FILE_DIR:${TARGET}>)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SFML::System> $<TARGET_FILE_DIR:${TARGET}>)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SFML::Window> $<TARGET_FILE_DIR:${TARGET}>)
endif()