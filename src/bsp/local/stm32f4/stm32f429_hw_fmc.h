/*
 * Copyright (C) 2022 Adrian Chadd <adrian@freebsd.org>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-Licence-Identifier: GPL-3.0-or-later
 */
#ifndef	__STM32F429_HW_FMC_H__
#define	__STM32F429_HW_FMC_H__

extern	bool stm32f429_hw_fmc_send_command(uint32_t command);
extern	bool stm32f429_hw_fmc_set_refresh_count(uint32_t refresh);
extern	bool stm32f429_hw_fmc_disable_write_protection(void);

#endif
