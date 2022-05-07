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
#ifndef	__KERN_CORE_LOGGING_H__
#define	__KERN_CORE_LOGGING_H__

typedef enum {
	KERN_LOG_LEVEL_NONE,
	KERN_LOG_LEVEL_CRIT,
	KERN_LOG_LEVEL_NOTICE,
	KERN_LOG_LEVEL_INFO,
	KERN_LOG_LEVEL_DEBUG
} kern_log_level_t;

struct kern_log_section {
	int level;
	char *name;
};

#define	LOGGING_DEFINE(label, def_name, def_level) \
	struct kern_log_section kern_log_section_##label = { \
		.level = def_level, \
		.name = def_name, \
	};

#define	LOGGING_EXT(label) \
	extern struct kern_log_section kern_log_section_##label;

#define	LOGGING_STRUCT(label) \
	kern_log_section_ ## label

#define KERN_LOG(l_label, l_level, fmt, ...) \
	if (LOGGING_STRUCT(l_label).level >= l_level) \
		kern_log(l_level, LOGGING_STRUCT(l_label).name, fmt, ##__VA_ARGS__)

extern	void kern_log(kern_log_level_t level, const char *label,
	    const char *fmt, ...);

#endif	/* __KERN_CORE_LOGGING_H__ */
