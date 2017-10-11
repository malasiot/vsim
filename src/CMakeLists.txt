set(UTIL_FILES
    ${SRC_FOLDER}/util/filesystem.cpp
    ${SRC_FOLDER}/util/dictionary.cpp
    ${SRC_FOLDER}/util/xml_sax_parser.cpp
#    ${SRC_FOLDER}/renderer/renderer.cpp
)

add_library(vsim_util ${UTIL_FILES})
target_link_libraries(vsim_util)


set(RENDERER_FILES
    ${SRC_FOLDER}/renderer/glsl.cpp
#    ${SRC_FOLDER}/renderer/renderer.cpp
)

add_library(vsim_renderer ${RENDERER_FILES})
target_link_libraries(vsim_renderer vsim_util ${ASSIMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARIES})
