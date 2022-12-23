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
#include <stdint.h>

/*
 * Note: MSB-first = 0x04c11db7
 * Note: LSB-first = 0xedb88320
 */

uint32_t
kern_crc32b(const char *s, size_t n)
{
	size_t i, j;
	uint32_t b, crc = 0xffffffff;
	char ch;

	for (i = 0; i < n; i++) {
		ch = s[i];
		for (j = 0; j < 8; j++) {
			b = (ch ^ crc) & 1;
			crc >>= 1;
			if (b) {
				crc = crc ^ 0xedb88320;
			}
			ch >>= 1;
		}
	}

	return (~crc);
}
