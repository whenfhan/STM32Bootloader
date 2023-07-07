# STM32BootLoader
本项目是在[stm32Flash](https://github.com/ARMinARM/stm32flash)基础上改写，并c++封装

## 编译环境
使用MinGW编译出来的库，由于只将stm32Flash中的C代码编译成动态库，所以动态库可供MinGW和MSVC使用``(编译时需要将CMAKE_GNUtoMS设置为ON，才能编译出MSVC需要的.lib文件)``

## 封装接口
``` cpp
class STM32Bootloader
{
public:
    STM32Bootloader();
    ~STM32Bootloader();
    int Open(const char *device);
    stm32_t *GetStm32Info() { return stm; }
    int ReadMemory(uint32_t address, uint32_t length, const char *filename);
    int WriteMemory(uint32_t address, const char *filename, bool verify=0);
    int EraseMemory(int first_page = 0, int num_pages = STM32_MASS_ERASE);
    int Close();
}
```
具体使用示例请参考test/main.cpp
