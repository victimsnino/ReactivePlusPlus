set(TARGET snake)

add_executable(${TARGET}
    main.cpp

    snake.cpp
    snake.hpp

    utils.hpp

    canvas.hpp
    canvas.cpp
)

target_link_libraries(${TARGET} PRIVATE rpp sfml-graphics)
set_target_properties(${TARGET} PROPERTIES FOLDER Examples/SFML)

if (WIN32)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-graphics> $<TARGET_FILE_DIR:${TARGET}>)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-system> $<TARGET_FILE_DIR:${TARGET}>)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sfml-window> $<TARGET_FILE_DIR:${TARGET}>)
endif()