add_executable(main main.cpp STM32Bootloader.cpp)
target_link_directories(main PRIVATE 
   .
   ${PROJECT_SOURCE_DIR}/include)

# 如果是MSVC，就用libstm32flash
if(MSVC)
   target_link_libraries(main PRIVATE libstm32flash)
endif()
   target_link_libraries(main PRIVATE stm32flash)

# 拷贝必要的dll文件/系统插件/到可执行文件目录下
add_custom_command(TARGET main POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
   $<TARGET_RUNTIME_DLLS:main> $<TARGET_FILE_DIR:main>
   COMMAND_EXPAND_LISTS
)