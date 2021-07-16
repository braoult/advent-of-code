/* SPDX-License-Identifier: GPL-2.0 */
/* circular list management.
 *
 * inspired from kernel's <linux/lists.h>
 */
#ifndef _BR_LIST_H
#define _BR_LIST_H

#include <stddef.h>

#define POISON_POINTER1 ((void *) 0x1)
#define POISON_POINTER2 ((void *) 0x2)

struct list_head {
    struct list_head *next, *prev;
};

#define container_of(ptr, type, member) ({                            \
            const typeof(((type *)0)->member) * __mptr = (ptr);         \
            (type *)((char *)__mptr - offsetof(type, member)); })

#define LIST_HEAD(name)                                 \
    struct list_head name = { &(name), &(name) }

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}
static inline void __list_join(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}
static inline void __list_del_entry(struct list_head *entry)
{
    __list_join(entry->prev, entry->next);
}


static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

static inline void list_del(struct list_head *entry)
{
    __list_del_entry(entry);
    entry->next = POISON_POINTER1;
    entry->prev = POISON_POINTER1;
}

static inline void list_replace(struct list_head *old,
				struct list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

static inline void list_swap(struct list_head *entry1,
			     struct list_head *entry2)
{
    struct list_head *pos = entry2->prev;

    list_del(entry2);
    list_replace(entry1, entry2);
    if (pos == entry1)
        pos = entry2;
    list_add(entry1, pos);
}

static inline int list_is_first(const struct list_head *list,
                                const struct list_head *head)
{
    return list->prev == head;
}

static inline int list_is_last(const struct list_head *list,
                               const struct list_head *head)
{
    return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline int list_is_singular(const struct list_head *head)
{
    return !list_empty(head) && (head->next == head->prev);
}

#define list_entry(ptr, type, member)           \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member)     \
    list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member)      \
    list_entry((ptr)->prev, type, member)

#define list_first_entry_or_null(ptr, type, member) ({                  \
            struct list_head *head__ = (ptr);                           \
            struct list_head *pos__ = READ_ONCE(head__->next);          \
            pos__ != head__ ? list_entry(pos__, type, member) : NULL;   \
        })

#define list_next_entry(pos, member)                            \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member)                            \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_for_each(pos, head)                                \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_continue(pos, head)                       \
    for (pos = pos->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head)                           \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_entry_is_head(pos, head, member)   \
    (&pos->member == (head))

#define list_for_each_entry(pos, head, member)                  \
    for (pos = list_first_entry(head, typeof(*pos), member);	\
         !list_entry_is_head(pos, head, member);                \
         pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, member))

#define list_for_each_entry_reverse(pos, head, member)          \
    for (pos = list_last_entry(head, typeof(*pos), member);     \
         !list_entry_is_head(pos, head, member);                \
         pos = list_prev_entry(pos, member))

#define list_for_each_entry_continue_reverse(pos, head, member) \
    for (pos = list_prev_entry(pos, member);			\
         !list_entry_is_head(pos, head, member);                \
         pos = list_prev_entry(pos, member))

#endif  /* _BR_LIST_H */
