add_executable(test_renderer test_renderer.cpp glfw_window.cpp trackball.cpp)
target_link_libraries(test_renderer vsim ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES})
