add_executable(test_renderer test_renderer.cpp)
target_link_libraries(test_renderer vsim_renderer ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES})
