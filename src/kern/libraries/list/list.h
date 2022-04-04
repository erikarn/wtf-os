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
#ifndef	__LIB_LIST_LIST_H__
#define	__LIB_LIST_LIST_H__

struct list_head;
struct list_node;

struct list_head {
	struct list_node *head, *tail;
};

struct list_node {
	struct list_node *prev, *next;
};

extern	void list_head_init(struct list_head *head);
extern	void list_node_init(struct list_node *node);

extern	void list_add_head(struct list_head *head, struct list_node *node);
extern	void list_add_tail(struct list_head *head, struct list_node *node);
extern	void list_delete(struct list_head *head, struct list_node *node);

#endif	/* __LIB_LIST_LIST_H__ */
