cmake_minimum_required(VERSION 3.6...3.14 FATAL_ERROR)
cmake_policy(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

project(rotor_cfd VERSION 0.1
			  DESCRIPTION "Computes thermal evolution using Navia Stokes equations"
			  LANGUAGES CXX)

set(CMAKE_CUDA_FLAGS_DEBUG "-G -g")

find_package(Vulkan REQUIRED)

include(FetchContent)

# Dependencies
FetchContent_Declare(vkbootstrap
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG v1.3.280
)
FetchContent_Declare(vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
)
FetchContent_Declare(SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.9
)
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.0.2
)
FetchContent_Declare(stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
)
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.0
)
FetchContent_Declare(fastgltf
  GIT_REPOSITORY https://github.com/spnda/fastgltf.git
  GIT_TAG main
)

FetchContent_MakeAvailable(
  vkbootstrap vma SDL2 glm fmt stb imgui fastgltf
)

# Add Vulkan library path to RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${Vulkan_LIBRARY_DIRS}")


# # Add tests
# if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
# 		add_subdirectory(tests)
# endif()

# Add main executable
set(CMAKE_CURRENT_SOURCE_DIR src)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(rotor_cfd ${SRC_FILES})
# Add imgui sources
target_sources(rotor_cfd PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Includes
target_include_directories(rotor_cfd PRIVATE
  ${stb_SOURCE_DIR}
  ${vma_SOURCE_DIR}/include
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# Link
target_link_libraries(rotor_cfd PRIVATE
  Vulkan::Vulkan
  vk-bootstrap::vk-bootstrap
  SDL2::SDL2 SDL2::SDL2main
  glm::glm
  fmt::fmt
  fastgltf::fastgltf
)
target_compile_definitions(rotor_cfd PRIVATE SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders")

# Set the directory for shaders
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SPIRV_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# Create the output directory for SPIR-V files
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

# List of shaders to compile
file(GLOB SHADERS "${SHADER_DIR}/*")

# Loop through each shader and add a custom command to compile it
foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME) # Get the shader name without extension
    set(SPIRV_FILE ${SPIRV_OUTPUT_DIR}/${SHADER_NAME}.spv) # Output SPIR-V file path

    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND glslc ${SHADER} -o ${SPIRV_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER} to ${SPIRV_FILE}"
        VERBATIM
    )

    # Add the SPIR-V file as a dependency for the executable
    list(APPEND SPIRV_FILES ${SPIRV_FILE})
endforeach()


target_compile_options(rotor_cfd PRIVATE
    -Wno-nullability-completeness
)


# Ensure shaders are compiled before building the executable
add_custom_target(main_shader_comp DEPENDS ${SPIRV_FILES})
add_dependencies(rotor_cfd main_shader_comp)