add_executable(petri_executor
    petri.cpp
)

target_link_libraries(petri_executor PRIVATE rpp)
set_target_properties(petri_executor PROPERTIES FOLDER Examples/rpp)
