# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS psu_init.h)
collect (PROJECT_LIB_HEADERS xis_plat.h)
collect (PROJECT_LIB_HEADERS xis_uart.h)
collect (PROJECT_LIB_HEADERS xis_gpio.h)
collect (PROJECT_LIB_HEADERS xis_qspi.h)

collect (PROJECT_LIB_SOURCES psu_init.c)
collect (PROJECT_LIB_SOURCES xis_plat.c)
collect (PROJECT_LIB_SOURCES xis_gpio.c)
collect (PROJECT_LIB_SOURCES xis_main.c)
collect (PROJECT_LIB_SOURCES xis_qspi.c)
collect (PROJECT_LIB_SOURCES xis_uart.c)

collect (PROJECT_LIB_SOURCES xis_translation_table_a53_64.S)
