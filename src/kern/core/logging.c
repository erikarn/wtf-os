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

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <hw/types.h>

#include <kern/libraries/screen/ansi.h>
#include <kern/libraries/string/string.h>
#include <kern/libraries/list/list.h>
#include <kern/libraries/container/container.h>

#include <kern/core/logging.h>
#include <kern/console/console.h>

void
kern_log(kern_log_level_t level, const char *label, const char *fmt, ...)
{
	va_list ap;

	switch (level) {
	case KERN_LOG_LEVEL_CRIT:
		console_printf("%s", ANSI_BRED);
		break;
	case KERN_LOG_LEVEL_INFO:
		console_printf("%s", ANSI_BYEL);
		break;
	case KERN_LOG_LEVEL_DEBUG:
		console_printf("%s", ANSI_BCYN);
		break;
	case KERN_LOG_LEVEL_NONE:
	default:
		console_printf("%s", ANSI_BWHT);
		break;
	}

	console_printf("[%s] ", label);

	va_start(ap, fmt);
	console_vprintf(fmt, ap);
	va_end(ap);

	console_printf("%s\n", ANSI_CRESET);
}
