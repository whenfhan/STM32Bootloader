#pragma once

#include <memory>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "stm32.h"
#include "parser.h"
#include "port.h"
}

class STM32Bootloader
{
public:
    STM32Bootloader();
    ~STM32Bootloader();
    int Open(const char *device);
    stm32_t *GetStm32Info() { return stm; }
    int ReadMemory(uint32_t address, uint32_t length, const char *filename);
    int WriteMemory(uint32_t address, const char *filename, bool verify = 0);
    int EraseMemory(int first_page = 0, int num_pages = STM32_MASS_ERASE);
    int Close();

private:
    void CalcStartEnd(uint32_t &start_addr, uint32_t readwrite_len);
    int is_addr_in_ram(uint32_t addr);
    int is_addr_in_flash(uint32_t addr);
    int is_addr_in_opt_bytes(uint32_t addr);
    int is_addr_in_sysmem(uint32_t addr);
    int flash_addr_to_page_floor(uint32_t addr);
    int flash_addr_to_page_ceil(uint32_t addr);
    uint32_t flash_page_to_addr(int page);

private:
    uint32_t start_addr_;
    uint32_t end_addr_;
    int first_page_;
    int num_pages_;

    const int retry_ = 10; // 重试次数

    /* device globals */
    stm32_t *stm = NULL;
    void *p_st = NULL;
    std::shared_ptr<parser_t> parser;
    struct port_interface *port = NULL;

    /* settings */
    struct port_options port_opts = {nullptr, SERIAL_BAUD_57600, "8e1", 0, STM32_MAX_RX_FRAME, STM32_MAX_TX_FRAME};
};