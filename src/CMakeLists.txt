set(UTIL_FILES
    ${SRC_FOLDER}/util/filesystem.cpp
    ${SRC_FOLDER}/util/dictionary.cpp
    ${SRC_FOLDER}/util/xml_sax_parser.cpp
    ${SRC_FOLDER}/util/strings.cpp
    ${SRC_FOLDER}/util/format.cpp

    ${SRC_FOLDER}/3rdparty/pugixml/pugixml.cpp
    ${SRC_FOLDER}/3rdparty/pugixml/pugixml.hpp
)


SET(OGL_SHADERS
    ${SRC_FOLDER}/renderer/shaders/skinning.vs
    ${SRC_FOLDER}/renderer/shaders/rigid.vs
    ${SRC_FOLDER}/renderer/shaders/phong.fs
    ${SRC_FOLDER}/renderer/shaders/flat.fs
    ${SRC_FOLDER}/renderer/shaders/gouraud.fs
    ${SRC_FOLDER}/renderer/shaders/textured.fs
)

add_executable(glsl2src ${SRC_FOLDER}/tools/glsl2src.cpp ${UTIL_FILES})
target_link_libraries(glsl2src)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp
    COMMAND glsl2src ${SRC_FOLDER}/renderer/shaders ${SRC_FOLDER}/renderer/shaders/shaders.xml ${CMAKE_BINARY_DIR}/vsim_shaders_library.cpp
    DEPENDS ${OGL_SHADERS} glsl2src
)

set(RENDERER_FILES
    ${SRC_FOLDER}/renderer/ogl_shaders.cpp
    ${SRC_FOLDER}/renderer/renderer.cpp

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

set(ENV_FILES
    ${SRC_FOLDER}/env/model.cpp
    ${SRC_FOLDER}/env/model_loader.cpp
    ${SRC_FOLDER}/env/assimp_loader.cpp
    ${SRC_FOLDER}/env/mesh.cpp
    ${SRC_FOLDER}/env/pose.cpp
    ${SRC_FOLDER}/env/camera.cpp
    ${SRC_FOLDER}/env/xml_loader.cpp

    ${INCLUDE_FOLDER}/env/scene.hpp
    ${INCLUDE_FOLDER}/env/model.hpp
    ${INCLUDE_FOLDER}/env/node.hpp
    ${INCLUDE_FOLDER}/env/camera.hpp
    ${INCLUDE_FOLDER}/env/mesh.hpp
    ${INCLUDE_FOLDER}/env/geometry.hpp
    ${INCLUDE_FOLDER}/env/material.hpp
    ${INCLUDE_FOLDER}/env/light.hpp
    ${INCLUDE_FOLDER}/env/body.hpp
    ${INCLUDE_FOLDER}/env/pose.hpp
    ${INCLUDE_FOLDER}/env/frame.hpp
    ${INCLUDE_FOLDER}/env/drawable.hpp
    ${INCLUDE_FOLDER}/env/environment.hpp
)

add_library(vsim ${UTIL_FILES} ${RENDERER_FILES} ${ENV_FILES})
target_link_libraries(vsim ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARY} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES} ${FREEIMAGE_LIBRARIES} ${FREETYPE_LIBRARIES})
