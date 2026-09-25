cmake_minimum_required(VERSION 4.3.3)
# TODO: Make this a search or wtv
# Also should probably verify the version
set(SLANGC_EXECUTABLE
    "/usr/local/bin/slangc"
)
find_program(
    SLANGC_EXECUTABLE
    NAMES
    slangc
)
if(NOT SLANGC_EXECUTABLE)
   message(FATAL_ERROR "Failed to locate slangc executable!")
endif()
set(SHADERS_OUT_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/bin"
)
set (SPIRV_PROFILE
    spirv_1_4 
)
function (add_slang_shader_target TARGET)
    cmake_parse_arguments (
        PARSE_ARGV 1
        SHADER          # prefix 
        ""              # options
        SOURCE          # single-value argument
        ENTRY_POINTS    # multi-value arguments
    )
    # Creates:
    # SHADER_SOURCE = '.../shaders/...'
    # SHADER_ENTRY_POINTS = entryPoint1;entryPoint2;entryPoint3...

    set(OUTPUT_FILE         "${SHADERS_OUT_DIR}/${TARGET}.spv")

    set(ENTRY_ARGS)
    foreach(ENTRY_POINT IN LISTS SHADER_ENTRY_POINTS)
        # Creates -entry;entryPoint1;-entry;entryPoint2;-entry;entryPoint3...
            list(APPEND ENTRY_ARGS -entry ${ENTRY_POINT})
    endforeach()

    # Add all the headers, st. changes in the header files cause recompilation of the shaders
    file(
        GLOB 
        SLANG_HEADERS 
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.slangh"
    )
    add_custom_command (
        OUTPUT  
            ${OUTPUT_FILE}

        #1: ensure the output directory exists
        COMMAND 
            ${CMAKE_COMMAND} -E make_directory "${SHADERS_OUT_DIR}"

        #2: Compile the shader to .spv
        COMMAND 
            ${SLANGC_EXECUTABLE} 
            ${SHADER_SOURCE} 
            -target spirv 
            -profile ${SPIRV_PROFILE}
            -emit-spirv-directly 
            -fvk-use-entrypoint-name ${ENTRY_ARGS} 
            -o ${OUTPUT_FILE}

        DEPENDS 
            ${SHADER_SOURCE}
            ${SLANG_HEADERS}

        COMMENT "Compiling Slang Shader ${TARGET} -> (${OUTPUT_FILE})"
        VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${OUTPUT_FILE})
endfunction()
