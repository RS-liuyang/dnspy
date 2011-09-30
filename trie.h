#ifndef ALGORITHM_TRIE_H
#define ALGORITHM_TRIE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Trie Trie;

/**
 * Create a new trie.
 *
 * @return                   Pointer to a new trie structure.
 */

Trie *trie_new(void);

/**
 * Destroy a trie.
 *
 * @param trie               The trie to destroy.
 */

void trie_free(Trie *trie);

/**
 * Insert a new key-value pair into a trie.
 *
 * @param trie               The trie.
 * @param key                The key to access the new value.
 * @param value              The value.
 */

void trie_insert(Trie *trie, char *key, void *value);

/**
 * Look up a value from its key in a trie.
 *
 * @param trie               The trie.
 * @param key                The key.
 * @return                   The value associated with the key, or NULL if
 *                           not found in the trie.
 */

void *trie_lookup(Trie *trie, char *key);

/**
 * Remove an entry from a trie.
 *
 * @param trie               The trie.
 * @param key                The key of the entry to remove.
 */

void trie_remove(Trie *trie, char *key);

/**
 * Find the number of entries in a trie.
 *
 * @param trie               The trie.
 * @return                   Count of the number of entries in the trie.
 */

int trie_num_entries(Trie *trie);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_TRIE_H */
