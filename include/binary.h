/*
  stm32flash - Open Source ST STM32 flash program for *nix
  Copyright (C) 2010 Geoffrey McRae <geoff@spacevs.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _PARSER_BINARY_H
#define _PARSER_BINARY_H

#include "parser.h"

void* binary_init();
parser_err_t binary_open(void *storage, const char *filename, const char write);
parser_err_t binary_close(void *storage);
unsigned int binary_base(void *storage);
unsigned int binary_size(void *storage);
parser_err_t binary_read(void *storage, void *data, unsigned int *len);
parser_err_t binary_write(void *storage, void *data, unsigned int len);

extern parser_t PARSER_BINARY;
#endif
