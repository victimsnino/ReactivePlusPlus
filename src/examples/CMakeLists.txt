add_subdirectory(rpp)

if(RPP_BUILD_QT_CODE)
    add_subdirectory(rppqt)
endif()

if (RPP_BUILD_GRPC_CODE)
    add_subdirectory(rppgrpc)
endif()
