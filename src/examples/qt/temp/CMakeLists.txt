SET(TARGET temp)
add_executable(${TARGET}
    ${TARGET}.cpp
)

target_link_libraries(${TARGET} PRIVATE rppqt ${RPP_QT_TARGET}::Widgets)
set_target_properties(${TARGET} PROPERTIES FOLDER Examples)

if (WIN32)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RPP_QT_TARGET}::Core> $<TARGET_FILE_DIR:${TARGET}>)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RPP_QT_TARGET}::Widgets> $<TARGET_FILE_DIR:${TARGET}>)
  add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RPP_QT_TARGET}::Gui> $<TARGET_FILE_DIR:${TARGET}>)
endif()