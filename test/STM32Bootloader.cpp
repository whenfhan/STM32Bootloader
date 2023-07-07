#include "STM32Bootloader.h"
#include <stdexcept>
#include <memory>

extern "C"
{
#include "init.h"
#include "utils.h"
#include "serial.h"
#include "stm32.h"
#include "parser.h"
#include "port.h"
#include "binary.h"
#include "hex.h"
}

STM32Bootloader::STM32Bootloader()
{
    // 只支持二进制格式
    parser = std::make_shared<parser_t>(parser_t{"Raw BINARY",
                                                 binary_init,
                                                 binary_open,
                                                 binary_close,
                                                 binary_base,
                                                 binary_size,
                                                 binary_read,
                                                 binary_write});
    p_st = parser->init();
    if (!p_st)
    {
        throw std::runtime_error("Failed to initialize parser");
    }
}

STM32Bootloader::~STM32Bootloader()
{
    if (p_st)
    {
        parser->close(p_st);
        p_st = NULL;
    }
}

int STM32Bootloader::Open(const char *device)
{
    port_opts.device = device;
    if (port_open(&port_opts, &port) != PORT_ERR_OK)
    {
        Close();
        return -1;
    }
    port->flush(port);
    stm = stm32_init(port, 1);
    if (stm == NULL)
    {
        Close();
        return -2;
    }
    return 0;
}

int STM32Bootloader::ReadMemory(uint32_t address, uint32_t length, const char *filename)
{
    CalcStartEnd(address, length);
    unsigned int max_len = port_opts.rx_frame_max;

    if (parser->open(p_st, filename, 1) != PARSER_ERR_OK)
    {
        return -1;
    }

    uint32_t addr = start_addr_;
    uint8_t buffer[256];
    for (uint32_t addr = start_addr_; addr < end_addr_;)
    {
        uint32_t left = end_addr_ - addr;
        uint32_t len = max_len > left ? left : max_len;
        if (stm32_read_memory(stm, addr, buffer, len) != STM32_ERR_OK)
        {
            return -2;
        }
        if (parser->write(p_st, buffer, len) != PARSER_ERR_OK)
        {
            return -3;
        }
        addr += len;
    }
    return 0;
}

int STM32Bootloader::WriteMemory(uint32_t address, const char *filename, bool verify)
{
    unsigned int max_wlen, max_rlen;
    max_wlen = port_opts.tx_frame_max - 2; /* skip len and crc */
    max_wlen &= ~3;                        /* 32 bit aligned */
    max_rlen = port_opts.rx_frame_max;
    max_rlen = max_rlen < max_wlen ? max_rlen : max_wlen;

    // 解析二进制文件
    if (parser->open(p_st, filename, 0) != PARSER_ERR_OK)
    {
        return -1;
    }
    uint32_t size = parser->size(p_st);
    CalcStartEnd(address, size);
    if (stm32_erase_memory(stm, first_page_, num_pages_) != STM32_ERR_OK)
    {
        return -2;
    }

    uint8_t buffer[256];
    int failed = 0; // 失败次数
    uint32_t offset = 0;
    uint32_t addr = start_addr_;
    while (addr < end_addr_ && offset < size)
    {
        uint32_t left = end_addr_ - addr;
        uint32_t len = max_wlen > left ? left : max_wlen;
        len = len > size - offset ? size - offset : len;
        unsigned int reqlen = len;

        if (parser->read(p_st, buffer, &len) != PARSER_ERR_OK)
        {
            return -3;
        }
        if (len == 0)
        {
            return -4;
        }

    again:
        if (stm32_write_memory(stm, addr, buffer, len) != STM32_ERR_OK)
        {
            return -5;
        }

        if (verify)
        {
            std::shared_ptr<uint8_t> compare(new uint8_t[len], std::default_delete<uint8_t[]>());
            unsigned int offset, rlen;

            offset = 0;
            while (offset < len)
            {
                rlen = len - offset;
                rlen = rlen < max_rlen ? rlen : max_rlen;
                if (stm32_read_memory(stm, addr + offset, compare.get() + offset, rlen) != STM32_ERR_OK)
                {
                    return -6;
                }
                offset += rlen;
            }

            for (int r = 0; r < len; ++r)
                if (buffer[r] != compare.get()[r])
                {
                    if (failed == retry_)
                    {
                        return -7;
                    }
                    ++failed;
                    goto again;
                }

            failed = 0;
        }

        addr += len;
        offset += len;

        if (len < reqlen) /* Last read already reached EOF */
            break;
    }
    return 0;
}

int STM32Bootloader::EraseMemory(int first_page, int num_pages)
{
    first_page_ = first_page;
    num_pages_ = num_pages;
    if (stm32_erase_memory(stm, first_page, num_pages) != STM32_ERR_OK)
    {
        return -1;
    }
    return 0;
}

int STM32Bootloader::Close()
{
    if (stm)
    {
        stm32_close(stm);
        stm = NULL;
    }
    if (port)
    {
        port->close(port);
        port = NULL;
    }
    return 0;
}

void STM32Bootloader::CalcStartEnd(uint32_t &start_addr, uint32_t readwrite_len)
{
    if (start_addr == 0)
        /* default */
        start_addr_ = stm->dev->fl_start;
    else if (start_addr == 1)
        /* if specified to be 0 by user */
        start_addr_ = 0;
    else
        start_addr_ = start_addr;

    if (is_addr_in_flash(start_addr_))
        end_addr_ = stm->dev->fl_end;
    else
    {
        // no_erase = 1;
        if (is_addr_in_ram(start_addr_))
            end_addr_ = stm->dev->ram_end;
        else if (is_addr_in_opt_bytes(start_addr_))
            end_addr_ = stm->dev->opt_end + 1;
        else if (is_addr_in_sysmem(start_addr_))
            end_addr_ = stm->dev->mem_end;
        else
        {
            /* Unknown territory */
            if (readwrite_len)
                end_addr_ = start_addr_ + readwrite_len;
            else
                end_addr_ = start_addr_ + sizeof(uint32_t);
        }
    }

    if (readwrite_len && (end_addr_ > start_addr_ + readwrite_len))
        end_addr_ = start_addr_ + readwrite_len;

    first_page_ = flash_addr_to_page_floor(start_addr_);
    if (!first_page_ && end_addr_ == stm->dev->fl_end)
        num_pages_ = STM32_MASS_ERASE;
    else
        num_pages_ = flash_addr_to_page_ceil(end_addr_) - first_page_;
}

int STM32Bootloader::is_addr_in_ram(uint32_t addr)
{
    return addr >= stm->dev->ram_start && addr < stm->dev->ram_end;
}

int STM32Bootloader::is_addr_in_flash(uint32_t addr)
{
    return addr >= stm->dev->fl_start && addr < stm->dev->fl_end;
}

int STM32Bootloader::is_addr_in_opt_bytes(uint32_t addr)
{
    /* option bytes upper range is inclusive in our device table */
    return addr >= stm->dev->opt_start && addr <= stm->dev->opt_end;
}

int STM32Bootloader::is_addr_in_sysmem(uint32_t addr)
{
    return addr >= stm->dev->mem_start && addr < stm->dev->mem_end;
}

/* returns the page that contains address "addr" */
int STM32Bootloader::flash_addr_to_page_floor(uint32_t addr)
{
    int page;
    uint32_t *psize;

    if (!is_addr_in_flash(addr))
        return 0;

    page = 0;
    addr -= stm->dev->fl_start;
    psize = stm->dev->fl_ps;

    while (addr >= psize[0])
    {
        addr -= psize[0];
        page++;
        if (psize[1])
            psize++;
    }

    return page;
}

/* returns the first page whose start addr is >= "addr" */
int STM32Bootloader::flash_addr_to_page_ceil(uint32_t addr)
{
    int page;
    uint32_t *psize;

    if (!(addr >= stm->dev->fl_start && addr <= stm->dev->fl_end))
        return 0;

    page = 0;
    addr -= stm->dev->fl_start;
    psize = stm->dev->fl_ps;

    while (addr >= psize[0])
    {
        addr -= psize[0];
        page++;
        if (psize[1])
            psize++;
    }

    return addr ? page + 1 : page;
}

/* returns the lower address of flash page "page" */
uint32_t STM32Bootloader::flash_page_to_addr(int page)
{
    int i;
    uint32_t addr, *psize;

    addr = stm->dev->fl_start;
    psize = stm->dev->fl_ps;

    for (i = 0; i < page; i++)
    {
        addr += psize[0];
        if (psize[1])
            psize++;
    }

    return addr;
}