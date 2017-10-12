set(UTIL_FILES
    ${SRC_FOLDER}/util/filesystem.cpp
    ${SRC_FOLDER}/util/dictionary.cpp
    ${SRC_FOLDER}/util/xml_sax_parser.cpp
)

add_library(vsim_util ${UTIL_FILES})
target_link_libraries(vsim_util)

SET(OGL_SHADERS
    ${SRC_FOLDER}/renderer/shaders/skinning.vs
    ${SRC_FOLDER}/renderer/shaders/rigid.vs
    ${SRC_FOLDER}/renderer/shaders/phong.fs
    ${SRC_FOLDER}/renderer/shaders/flat.fs
    ${SRC_FOLDER}/renderer/shaders/gouraud.fs
)

add_executable(glsl2src ${SRC_FOLDER}/tools/glsl2src.cpp)
target_link_libraries(glsl2src vsim_util)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp
    COMMAND glsl2src ${SRC_FOLDER}/renderer/shaders ${SRC_FOLDER}/renderer/shaders/shaders.xml ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp
    DEPENDS ${OGL_SHADERS} glsl2src
)


include_directories(${CMAKE_BINARY_DIR})

set(RENDERER_FILES
    ${SRC_FOLDER}/renderer/ogl_shaders.cpp
    ${SRC_FOLDER}/renderer/renderer.cpp

    ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp

)

add_library(vsim_renderer ${RENDERER_FILES})
target_link_libraries(vsim_renderer vsim_util ${ASSIMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES})
