set(UTIL_FILES
    ${SRC_FOLDER}/util/filesystem.cpp
    ${SRC_FOLDER}/util/dictionary.cpp
    ${SRC_FOLDER}/util/xml_sax_parser.cpp
    ${SRC_FOLDER}/util/strings.cpp
    ${SRC_FOLDER}/util/format.cpp
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


set(RENDERER_FILES
    ${SRC_FOLDER}/renderer/ogl_shaders.cpp
    ${SRC_FOLDER}/renderer/renderer.cpp
    ${SRC_FOLDER}/renderer/scene.cpp
    ${SRC_FOLDER}/renderer/scene_loader.cpp
    ${SRC_FOLDER}/renderer/assimp_loader.cpp
    ${SRC_FOLDER}/renderer/renderer_impl.cpp
    ${SRC_FOLDER}/renderer/tools.cpp

    ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp
)

file (GLOB SOIL_SOURCES  ${SRC_FOLDER}/renderer/soil/*.c)
file (GLOB SOIL_HEADERS  ${SRC_FOLDER}/renderer/soil/*.h)
add_library (SOIL STATIC ${SOIL_SOURCES} ${SOIL_HEADERS})
target_link_libraries (SOIL ${OPENGL_LIBRARIES})


add_library(vsim_renderer ${RENDERER_FILES})
target_link_libraries(vsim_renderer vsim_util SOIL ${ASSIMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES})
