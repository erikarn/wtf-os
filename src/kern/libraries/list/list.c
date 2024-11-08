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
#include <stdbool.h>

#include <kern/libraries/list/list.h>
#include <kern/libraries/container/container.h>

void
list_head_init(struct list_head *head)
{
	head->head = head->tail = NULL;
}

void
list_node_init(struct list_node *node)
{
	node->prev = node->next = NULL;
}

void
list_add_head(struct list_head *head, struct list_node *node)
{
	node->prev = NULL;

	if (head->head != NULL) {
		/* There's an item on the head */
		node->next = head->head;
		head->head->prev = node;
		head->head = node;
	} else {
		/* List is empty */
		node->next = NULL;
		/* Also assume tail is NULL here */
		head->head = node;
		head->tail = node;
	}
}

void
list_add_tail(struct list_head *head, struct list_node *node)
{
	node->next = NULL;

	if (head->tail != NULL) {
		node->prev = head->tail;
		head->tail->next = node;
		head->tail = node;
	} else {
		/* List is empty */
		node->prev = NULL;
		/* Also assume head is NULL here */
		head->head = node;
		head->tail = node;
	}
}

void
list_add_before(struct list_head *head, struct list_node *before,
    struct list_node *node)
{
	node->next = before;
	node->prev = before->prev;
	if (before->prev != NULL) {
		before->prev->next = node;
	} else {
		/* It's at the beginning of the list */
		head->head = node;
	}
	before->prev = node;
}

void
list_add_after(struct list_head *head, struct list_node *after,
    struct list_node *node)
{
	node->prev = after;
	node->next = after->next;
	if (after->next != NULL) {
		after->next->prev = node;
	} else {
		/* It's at the end of the list */
		head->tail = node;
	}
	after->next = node;
}


void
list_delete(struct list_head *head, struct list_node *node)
{
	if (head->head == node)
		head->head = node->next;
	if (head->tail == node)
		head->tail = node->prev;

	if (node->next != NULL) {
		node->next->prev = node->prev;
	}

	if (node->prev != NULL) {
		node->prev->next = node->next;
	}
	node->next = NULL;
	node->prev = NULL;
}

bool
list_is_empty(const struct list_head *head)
{

	return (head->head == NULL);
}

struct list_node *
list_get_head(struct list_head *head)
{

	return (head->head);
}

struct list_node *
list_get_tail(struct list_head *head)
{

	return (head->tail);
}
