
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "radix_tree.h"

#include <stdio.h>

#define	INADDR_NONE		((in_addr_t) 0xffffffff)

intptr_t
rs_atoi(u_char *line, size_t n)
{
	intptr_t  value;

    if (n == 0) {
        return -1;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return -1;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return -1;

    } else {
        return value;
    }
}

u_char *
strlchr(u_char *p, u_char *last, u_char c)
{
    while (p < last) {

        if (*p == c) {
            return p;
        }

        p++;
    }
    return NULL;
}

in_addr_t
inet_addr(u_char *text, size_t len)
{
    u_char      *p, c;
    in_addr_t    addr;
    intptr_t   octet, n;

    addr = 0;
    octet = 0;
    n = 0;

    for (p = text; p < text + len; p++) {

        c = *p;

        if (c >= '0' && c <= '9') {
            octet = octet * 10 + (c - '0');
            continue;
        }

        if (c == '.' && octet < 256) {
            addr = (addr << 8) + octet;
            octet = 0;
            n++;
            continue;
        }

        return INADDR_NONE;
    }

    if (n != 3) {
        return INADDR_NONE;
    }

    if (octet < 256) {
        addr = (addr << 8) + octet;
        return htonl(addr);
    }

    return INADDR_NONE;
}

intptr_t
ptocidr(u_char *text, in_cidr_t *cidr)
{
    u_char      *addr, *mask, *last;
    size_t       len;
    intptr_t    shift;

    addr = text;
    last = addr + strlen((const char*)text);

    mask = strlchr(addr, last, '/');
    len = (mask ? mask : last) - addr;

    cidr->addr = inet_addr(addr, len);

    if (cidr->addr != INADDR_NONE)
    {
        if (mask == NULL)
        {
            cidr->mask = 0xffffffff;
            return 1;
        }
    }else
    {
    	return -1;
    }

    mask++;

    shift = rs_atoi(mask, last - mask);

    if (shift == -1) {
        return -1;
    }

	if (shift) {
		cidr->mask = htonl((uintptr_t) (0 - (1 << (32 - shift))));

	} else {
		/* x86 compilers use a shl instruction that shifts by modulo 32 */
		cidr->mask = 0;
	}

	if (cidr->addr == (cidr->addr & cidr->mask)) {
		return 1;
	}

	cidr->addr &= cidr->mask;

	return 1;

}



radix_tree_t *
radix_tree_create(void)
{
    //uint32_t		key, mask, inc;
    radix_tree_t	*tree;

    tree = (radix_tree_t*)malloc(sizeof(radix_tree_t));
    if (tree == NULL) {
        return NULL;
    }

    tree->free = NULL;
    tree->start = NULL;
    tree->size = 0;

    tree->root = (radix_node_t*)malloc(sizeof(radix_node_t));
    if (tree->root == NULL) {
        return NULL;
    }

    tree->root->right = NULL;
    tree->root->left = NULL;
    tree->root->parent = NULL;
    tree->root->value = RADIX_NO_VALUE;

    return tree;
 }


intptr_t
radix32tree_insert(radix_tree_t *tree, uint32_t key, uint32_t mask,
    uintptr_t value)
{
    uint32_t           bit;
    radix_node_t  *node, *next;

    bit = 0x80000000;

    node = tree->root;
    next = tree->root;

    while (bit & mask) {
        if (key & bit) {
            next = node->right;

        } else {
            next = node->left;
        }

        if (next == NULL) {
            break;
        }

        bit >>= 1;
        node = next;
    }

    if (next) {
        if (node->value != RADIX_NO_VALUE) {
            return -2;
        }

        node->value = value;
        return 1;
    }

    while (bit & mask) {

    	next = (radix_node_t*)malloc(sizeof(radix_node_t));
        if (next == NULL) {
            return -1;
        }

        next->right = NULL;
        next->left = NULL;
        next->parent = node;
        next->value = RADIX_NO_VALUE;

        if (key & bit) {
            node->right = next;

        } else {
            node->left = next;
        }

        bit >>= 1;
        node = next;
    }

    node->value = value;

    return 1;
}


intptr_t
radix32tree_delete(radix_tree_t *tree, uint32_t key, uint32_t mask)
{
    uint32_t           bit;
    radix_node_t  *node;

    bit = 0x80000000;
    node = tree->root;

    while (node && (bit & mask)) {
        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    if (node == NULL) {
        return -1;
    }

    if (node->right || node->left) {
        if (node->value != RADIX_NO_VALUE) {
            node->value = RADIX_NO_VALUE;
            return 1;
        }

        return -1;
    }

    for ( ;; ) {
        if (node->parent->right == node) {
            node->parent->right = NULL;

        } else {
            node->parent->left = NULL;
        }

        node->right = tree->free;
        tree->free = node;

        node = node->parent;

        if (node->right || node->left) {
            break;
        }

        if (node->value != RADIX_NO_VALUE) {
            break;
        }

        if (node->parent == NULL) {
            break;
        }
    }

    return 1;
}

uintptr_t
radix32tree_find(radix_tree_t *tree, uint32_t key)
{
    uint32_t           bit;
    uintptr_t          value;
    radix_node_t  *node;

    bit = 0x80000000;
    value = RADIX_NO_VALUE;
    node = tree->root;

    while (node) {
        if (node->value != RADIX_NO_VALUE) {
            value = node->value;
        }

        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    return value;
}
