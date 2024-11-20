cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME MarchingCubeSDF)

project(${PROJECT_NAME} VERSION 1.0)

# Set the output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

# GLAD
set(GLAD_PATH dependency/glad)
set(GLAD_FILES ${GLAD_PATH}/include/glad/glad.h
               ${GLAD_PATH}/src/glad.c)
source_group(lib\\glad FILES ${GLAD_FILES})

# GLFW
set(GLFW_PATH dependency/glfw)
file(GLOB GLFW_FILES ${GLFW_PATH}/include/GLFW/*.h)
source_group(lib\\GLFW FILES ${GLFW_FILES})

set(GLFW_INSTALL OFF CACHE BOOL "GLFW Ignore Install")
set(GLFW_BUILD_SHARED_LIBRARY OFF CACHE BOOL "Do not build GLFW dll ")

# IMGUI
set(IMGUI_PATH dependency/imgui)
file(GLOB IMGUI_FILES ${IMGUI_PATH}/include/imgui/*.h 
                      ${IMGUI_PATH}/src/*.cpp)
source_group(lib\\imgui FILES ${IMGUI_FILES})

#tinyobjloader
set(TINYOBJLOADER_PATH dependency/tinyobjloader)
set(TINYOBJLOADER_FILES ${TINYOBJLOADER_PATH}/tiny_obj_loader.h)
source_group(lib\\tinyobjloader FILES ${TINYOBJLOADER_FILES})

# glm
set(GLM_PATH dependency/glm)
file(GLOB_RECURSE GLM_FILES ${GLM_PATH}/include/glm/*.hpp
                            ${GLM_PATH}/include/glm/*.inl)
source_group(lib\\glm FILES ${GLM_FILES})

set(SOURCE_FILES
     code/main.cpp
     code/common.h
     code/common.cpp
     code/vector.h
     code/vector.cpp
     code/aabb.h
     code/aabb.cpp
     code/geometry_algorithm.h
     code/geometry_algorithm.cpp
     code/window.h
     code/window.cpp
     code/gui.h
     code/gui.cpp
     code/gl.h
     code/gl.cpp
     code/render.h
     code/render.cpp
     code/render_primitive.h
	 code/render_primitive.cpp
     code/camera.h
     code/camera.cpp
     code/obj.h
     code/obj.cpp
     code/sdf_obj.h
     code/sdf_obj.cpp
     code/marching_cubes.h
     code/marching_cubes.cpp)
source_group(source FILES ${SOURCE_FILES})

set(RESOURCE_FILES
	resource/object.vs
	resource/object.fs
    resource/marching_cubes.vs
    resource/marching_cubes.gs
    resource/marching_cubes.fs
	)
source_group(resource FILES ${RESOURCE_FILES})

add_executable(${PROJECT_NAME}
               ${SOURCE_FILES}
               ${RESOURCE_FILES}
               ${GLAD_FILES}
               ${GLFW_FILES}
               ${IMGUI_FILES}
               ${TINYOBJLOADER_FILES}
               ${GLM_FILES})

add_subdirectory(dependency/glfw)

target_include_directories(${PROJECT_NAME} PUBLIC ${GLAD_PATH}/include
												  ${GLFW_PATH}/include
                                                  ${IMGUI_PATH}/include
                                                  ${TINYOBJLOADER_PATH}
                                                  ${GLM_PATH}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)  # use glfw in static library 

if(MSVC)
	set(INSTALL_ADDITIONAL_PATH "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>")
	# https://stackoverflow.com/questions/23950887/does-cmake-offer-a-method-to-set-the-working-directory-for-a-given-build-system
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INSTALL_ADDITIONAL_PATH}")
	target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
else()
	set(INSTALL_ADDITIONAL_PATH "")
endif()

# move resource files into the executable location
install(DIRECTORY "resource" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${INSTALL_ADDITIONAL_PATH}")