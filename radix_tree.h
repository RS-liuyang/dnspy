

#ifndef _RADIX_TREE_H_INCLUDED_
#define _RADIX_TREE_H_INCLUDED_

#include <stddef.h>
#include <inttypes.h>

#define RADIX_NO_VALUE   (uintptr_t) -1

typedef struct radix_node_s  radix_node_t;

struct radix_node_s {
    radix_node_t  *right;
    radix_node_t  *left;
    radix_node_t  *parent;
    uintptr_t     value;
};

typedef struct {
    radix_node_t  *root;
    radix_node_t  *free;
    char              *start;
    size_t             size;
} radix_tree_t;

#include <netinet/in.h>
typedef struct {
    in_addr_t                 addr;
    in_addr_t                 mask;
}in_cidr_t;

intptr_t ptocidr(u_char *text, in_cidr_t *cidr);

radix_tree_t *radix_tree_create(void);
intptr_t radix32tree_insert(radix_tree_t *tree,
    uint32_t key, uint32_t mask, uintptr_t value);
intptr_t radix32tree_delete(radix_tree_t *tree,
    uint32_t key, uint32_t mask);
uintptr_t radix32tree_find(radix_tree_t *tree, uint32_t key);


#endif /* _RADIX_TREE_H_INCLUDED_ */
