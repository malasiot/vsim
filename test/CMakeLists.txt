add_executable(test_renderer test_renderer.cpp glfw_window.cpp trackball.cpp)
target_link_libraries(test_renderer vsim ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES} ${LUA_LIBRARIES})


add_executable(test_lua test_lua.cpp)
target_link_libraries(test_lua ${LUA_LIBRARIES})

add_executable(test_bullet test_bullet.cpp glfw_window.cpp )
target_link_libraries(test_bullet ${BULLET_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES})
