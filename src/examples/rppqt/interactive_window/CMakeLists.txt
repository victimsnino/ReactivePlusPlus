SET(TARGET interactive_window)
add_executable(${TARGET}
    ${TARGET}.cpp
)

target_link_libraries(${TARGET} PRIVATE rppqt ${RPP_QT_TARGET}::Widgets)
set_target_properties(${TARGET} PROPERTIES FOLDER Examples/rppqt)

rpp_add_qt_support_to_executable(${TARGET})