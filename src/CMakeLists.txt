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
    ${SRC_FOLDER}/renderer/shaders/textured.fs
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

    ${SRC_FOLDER}/renderer/ftgl/vector.c
    ${SRC_FOLDER}/renderer/ftgl/texture-font.c
    ${SRC_FOLDER}/renderer/ftgl/texture-atlas.c
    ${SRC_FOLDER}/renderer/ftgl/edtaa3func.c
    ${SRC_FOLDER}/renderer/ftgl/distance-field.c
    ${SRC_FOLDER}/renderer/ftgl/platform.c
    ${SRC_FOLDER}/renderer/ftgl/utf8-utils.c

    ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp
)

add_library(vsim_renderer ${RENDERER_FILES})
target_link_libraries(vsim_renderer vsim_util ${ASSIMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES} ${FREEIMAGE_LIBRARIES} ${FREETYPE_LIBRARIES})
