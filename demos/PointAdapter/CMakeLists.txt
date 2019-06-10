project(PointAdapter)

set(Demo_PointerAdapter_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/super4pcs_adapter_test.cc
)

add_executable(${PROJECT_NAME} ${Demo_PointerAdapter_SRC})
target_link_libraries(${PROJECT_NAME} opengr_utils opengr_io opengr_accel opengr_algo)
add_dependencies(${PROJECT_NAME} opengr)
if(OpenGR_USE_CHEALPIX)
    target_link_libraries(${PROJECT_NAME} chealpix)
endif(OpenGR_USE_CHEALPIX)
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin  )
