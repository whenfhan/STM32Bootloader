set(CMAKE_GNUtoMS ON)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)
add_library(stm32flash SHARED ${SRCS})