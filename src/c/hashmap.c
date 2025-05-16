/** @file hashmap.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


/* ----------------------------- General ----------------------------- */

/* This MUST be a power of 2. */
#define INITIAL_CAP  16
#define LOAD_FACTOR  0.7f

/* ----------------------------- HashMap ----------------------------- */

/* Perfom a hashmap `action` under mutex protection.  Note that this also asserts that the map ptr is valid and that the map is in a valid state. */
#define HASHMAP_MUTEX_ACTION(...)  \
  DO_WHILE(                        \
    ASSERT(map);                   \
    mutex_action(&map->globmutex,  \
      ASSERT(map->cap);            \
      ASSERT(map->buckets);        \
      DO_WHILE(__VA_ARGS__);       \
    );                             \
  )

#define HASHMAP_ITER(__map, __itername, __nodename, ...)              \
  DO_WHILE(                                                           \
    for (int __itername=0; __itername<(__map)->cap; ++__itername)  {  \
      HashNode *__nodename = (__map)->buckets[__itername];            \
      DO_WHILE(__VA_ARGS__);                                          \
    }                                                                 \
  )

/* ----------------------------- HashMapNum ----------------------------- */

/* Perfom a hashmap `action` under mutex protection.  Note that this also asserts that the map ptr is valid and that the map is in a valid state. */
#define HASHMAPNUM_MUTEX_ACTION(...)  \
  DO_WHILE(                           \
    ASSERT(map);                      \
    mutex_action(&map->mutex,         \
      ASSERT(map->cap);               \
      ASSERT(map->buckets);           \
      DO_WHILE(__VA_ARGS__);          \
    );                                \
  )

#define HASHMAPNUM_ITER(__map, __itername, __nodename, ...)           \
  DO_WHILE(                                                           \
    for (int __itername=0; __itername<(__map)->cap; ++__itername)  {  \
      HashNodeNum *__nodename = (__map)->buckets[__itername];         \
      DO_WHILE(__VA_ARGS__);                                          \
    }                                                                 \
  )

/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


/* ----------------------------- HashMap ----------------------------- */

struct HashNode {
  Ulong hash;      /* Cached hash value, used when resizing so we can avoid recalculating every time. */
  char *key;       /* The key of this hashmap node. */
  void *value;     /* Ptr to the value this node holds. */
  HashNode *next;  /* Ptr to the next node, used when there are conflicts. */
};

struct HashMap {
  /* This is where the data is stored, in `buckets` where each bucket has a linked list, when there are collisions. */
  HashNode **buckets;
  
  /* The currently allocated size of `buckets`, as a whole. */
  int cap;
  
  /* Current number of buckets with data in them. */
  int size;

  /* A callback used to free the value of a `HashNode`, so that deallocation can happen when needed. */
  FreeFuncPtr free_value;

  /* This is locked when we resize the hashmap, so that we ensure singular thread resizeing. */
  mutex_t globmutex;
};

/* ----------------------------- HashMapNum ----------------------------- */

struct HashNodeNum {
  Ulong key;
  void *value;
  HashNodeNum *next;
};

struct HashMapNum {
  HashNodeNum **buckets;
  int cap;
  int size;

  FreeFuncPtr free_value;
  
  mutex_t mutex;
};


/* ---------------------------------------------------------- Function's ---------------------------------------------------------- */


/* ----------------------------- HashMap ----------------------------- */

/* `INTERNAL`  Create a `djb2` hash from `str`. */
static inline Ulong hash_djb2(const char *str) {
  ASSERT(str);
  Ulong hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = (((hash << 5) + hash) + c);
  }
  return hash;
}

/* `INTERNAL`  Get the current `cap` or `size` of `map`, or both, NULL can be passed to one, but not both at the same time. */
static inline void hashmap_get_data(HashMap *const map, int *const cap, int *const size) {
  /* Ensure at least one of the params are valid. */
  ASSERT(cap || size);
  /* Perform the retrival under mutex protection. */
  HASHMAP_MUTEX_ACTION(
    ASSIGN_IF_VALID(cap, map->cap);
    ASSIGN_IF_VALID(size, map->size);
  );
}

/* `INTERNAL`  Free a `HashNode` structure. */
static inline void hashmap_free_node(HashMap *const map, HashNode *const node) {
  ASSERT(map);
  /* Ensure map is in a valid state. */
  ASSERT(map->cap);
  ASSERT(map->buckets);
  ASSERT(node);
  /* If a callback has been set to free the value of node, then call it. */
  CALL_IF_VALID(map->free_value, node->value);
  free(node->key);
  free(node);
}

/* Create a `new hashmap`. */
HashMap *hashmap_create(void) {
  HashMap *map    = xmalloc(sizeof(*map));
  map->cap        = INITIAL_CAP;
  map->size       = 0;
  map->buckets    = xcalloc(map->cap, sizeof(HashNode *));
  map->free_value = NULL;
  mutex_init(&map->globmutex, NULL);
  return map;
}

/* Free a hashmap structure. */
void hashmap_free(HashMap *const map) {
  HashNode *next;
  /* Ensure thread safe operation, even tough this should not ever be needed. */
  HASHMAP_MUTEX_ACTION(
    HASHMAP_ITER(map, i, node,
      while (node) {
        next = node->next;
        hashmap_free_node(map, node);
        node = next;
      }
    );
  );
  mutex_destroy(&map->globmutex);
  free(map->buckets);
  free(map);
}

/* Create a new allocated `HashMap` structure that uses `freefunc` to free the values of nodes when the map is freed. */
HashMap *hashmap_create_wfreefunc(FreeFuncPtr freefunc) {
  HashMap *map = hashmap_create();
  hashmap_set_free_value_callback(map, freefunc);
  return map;
}

/* Set the function that should be used to free HashNode's value.  Signature should ba `void foo(void *)`. */
void hashmap_set_free_value_callback(HashMap *const map, FreeFuncPtr callback) {
  HASHMAP_MUTEX_ACTION(
    map->free_value = callback;
  );
}

/* `INTERNAL`  Resize `map`, this is called when `map->cap` goes above the set `LOAD_FACTOR`. */
static void hashmap_resize(HashMap *const map) {
  /* Ensure the ptr to the map is valid. */
  ASSERT(map);
  /* And that the map is in a valid state. */
  ASSERT(map->cap);
  ASSERT(map->buckets);
  int newcap;
  Ulong index;
  HashNode **newbuckets, *next;
  newcap = (map->cap * 2);
  newbuckets = xcalloc(newcap, sizeof(HashNode *));
  /* Recalculate all entries. */
  HASHMAP_ITER(map, i, node,
    while (node) {
      next              = node->next;
      index             = (node->hash & (newcap - 1));
      node->next        = newbuckets[index];
      newbuckets[index] = node;
      node              = next;
    }  
  );
  free(map->buckets);
  map->buckets = newbuckets;
  map->cap     = newcap;
}

/* `INTERNAL`  Insert a entry into the map without locking the mutex.  Used internaly when mutex is already locked. */
static void hashmap_insert_unlocked(HashMap *const map, const char *const restrict key, void *value) {
  ASSERT(map);
  ASSERT(key);
  ASSERT(value);
  /* The calculated hash of `key`. */
  Ulong hash;
  /* Index in the buckets of map, based on the current cap of the map. */
  Ulong index;
  /* Ptr to a intenal node strucure. */
  HashNode *node;
  /* Resize the map if we excede the load factor. */
  if (((float)(map->size + 1) / map->cap) > LOAD_FACTOR) {
    hashmap_resize(map);
  }
  hash  = hash_djb2(key);
  index = (hash & (map->cap - 1));
  node  = map->buckets[index];
  while (node) {
    if (strcmp(node->key, key) == 0) {
      /* If there is a free function set, then use it to free the value before overwriting it. */
      CALL_IF_VALID(map->free_value, node->value);
      node->value = value;
      return;
    }
    node = node->next;
  }
  /* When there is no match already in the map, add it. */
  node        = xmalloc(sizeof(*node));
  node->hash  = hash;
  node->key   = copy_of(key);
  node->value = value;
  /* Insert the newly made node at the start of the bucket. */
  node->next = map->buckets[index];
  map->buckets[index] = node;
  ++map->size;
}

/* Insert a entry into `map` with `key` and `value`. */
void hashmap_insert(HashMap *const map, const char *const restrict key, void *value) {
  ASSERT(key);
  ASSERT(value);
  /* Ensure thread-safe insertion. */
  HASHMAP_MUTEX_ACTION(
    hashmap_insert_unlocked(map, key, value);
  );
}

/* `INTERNAL`  Get the node tied to `key`, if it exists. */
static HashNode *hashmap_get_node_unlocked(HashMap *const map, const char *key) {
  ASSERT(map);
  ASSERT(map->cap);
  ASSERT(map->buckets);
  ASSERT(key);
  Ulong index = (hash_djb2(key) & (map->cap - 1));
  HashNode *node = map->buckets[index];
  while (node) {
    if (strcmp(node->key, key) == 0) {
      return node;
    }
    node = node->next;
  }
  return NULL;
}

/* `INTERNAL`  Get the user value tied to `key`, if it exists.  Note that this performs all operation without mutex protection. */
static void *hashmap_get_unlocked(HashMap *const map, const char *key) {
  ASSERT(map);
  ASSERT(map->cap);
  ASSERT(map->buckets);
  ASSERT(key);
  Ulong index=(hash_djb2(key) & (map->cap - 1));
  HashNode *node = map->buckets[index];
  while (node) {
    if (strcmp(node->key, key) == 0) {
      return node->value;
    }
    node = node->next;
  }
  return NULL;
}

/* Retrieve the `value` of a entry using the key of that entry, if any.  Otherwise, returns `NULL`. */
void *hashmap_get(HashMap *const map, const char *key) {
  ASSERT(key);
  void *ret;
  HASHMAP_MUTEX_ACTION(
    ret = hashmap_get_unlocked(map, key);
  );
  return ret;
}

/* Remove one entry from the hash map. */
void hashmap_remove(HashMap *const map, const char *key) {
  ASSERT(key);
  Ulong index;
  HashNode *node;
  HashNode *prev = NULL;
  /* Ensure thread-safe removal. */
  HASHMAP_MUTEX_ACTION(
    index = (hash_djb2(key) & (map->cap - 1));
    node = map->buckets[index];
    while (node) {
      /* Found the entry. */
      if (strcmp(node->key, key) == 0) {
        /* If the entry to erase is the not the first entry. */
        if (prev) {
          prev->next = node->next;
        }
        /* Otherwise, when its the first entry. */
        else {
          map->buckets[index] = node->next;
        }
        hashmap_free_node(map, node);
        --map->size;
        break;
      }
      prev = node;
      node = node->next;
    }
  );
}

/* Get the `size` of a `HashMap` structure in a `thread-safe` manner. */
int hashmap_size(HashMap *const map) {
  ASSERT(map);
  int size;
  hashmap_get_data(map, NULL, &size);
  return size;
}

/* Get the `capacity` of a `HashMap` structure in a `thread-safe` manner. */
int hashmap_cap(HashMap *const map) {
  ASSERT(map);
  int cap;
  hashmap_get_data(map, &cap, NULL);
  return cap;
}

/* Perform some action on all entries in the map.  Its importent to not run other hashmap
 * functions inside `action`, as this is thread-safe, and will cause a deadlock. */
void hashmap_forall(HashMap *const map, void (*action)(const char *const restrict key, void *value)) {
  ASSERT(action);
  /* Ensure thread-safe operation. */
  HASHMAP_MUTEX_ACTION(
    HASHMAP_ITER(map, i, node,
      while (node) {
        action(node->key, node->value);
        node = node->next;
      }
    );
  );
}

/* Clear and return `map` to original state when created. */
void hashmap_clear(HashMap *const map) {
  HashNode *next;
  HASHMAP_MUTEX_ACTION(
    /* Free all entries. */
    HASHMAP_ITER(map, i, node,
      while(node) {
        next = node->next;
        hashmap_free_node(map, node);
        node = next;
      }
    );
    /* Free the buckets. */
    free(map->buckets);
    /* Reallocate the buckets. */
    map->size = 0;
    map->cap  = INITIAL_CAP;
    map->buckets = xcalloc(map->cap, _PTRSIZE);
  );
}

/* Move all entries in `src` to `dst`.  Meaning dst now own the value ptr's, this is why we will
 * also set the free value function in `src` to `NULL`.  Meaning that `src` should be discarded. */
void hashmap_append(HashMap *const dst, HashMap *const src) {
  ASSERT(dst);
  ASSERT(src);
  mutex_action(&dst->globmutex, mutex_action(&src->globmutex,
    /* Ensure both maps are in a valid state. */
    ASSERT(dst->cap);
    ASSERT(dst->buckets);
    ASSERT(src->cap);
    ASSERT(src->buckets);
    HASHMAP_ITER(src, i, node,
      while (node) {
        hashmap_insert_unlocked(dst, node->key, node->value);
        node = node->next;
      }
    );
    src->free_value = NULL;
  ););
}

/* Same as `hashmap_append()` but performs `existing_action()` if a node's key already exists in the dst map. */
void hashmap_append_waction(HashMap *const dst, HashMap *const src, void (*existing_action)(void *dstnodevalue, void *srcnodevalue)) {
  ASSERT(dst);
  ASSERT(src);
  HashNode *dstnode;
  mutex_action(&dst->globmutex, mutex_action(&src->globmutex,
    /* Ensure both maps are in a valid state. */
    ASSERT(dst->cap);
    ASSERT(dst->buckets);
    ASSERT(src->cap);
    ASSERT(src->buckets);
    HASHMAP_ITER(src, i, node,
      while (node) {
        dstnode = hashmap_get_node_unlocked(dst, node->key);
        if (!dstnode) {
          hashmap_insert_unlocked(dst, node->key, node->value);
        }
        else {
          existing_action(dstnode->value, node->value);
        }
        node = node->next;
      }
    );
    src->free_value = NULL;
  ););
}

/* ----------------------------- HashMapNum ----------------------------- */

/* `INTERNAL`  Get the current `cap` or `size` of `map`, or both, NULL can be passed to one, but not both at the same time. */
static inline void hashmapnum_get_data(HashMapNum *const map, int *const cap, int *const size) {
  /* Ensure at least one of the params are valid. */
  ASSERT(cap || size);
  /* Perform the retrival under mutex protection. */
  HASHMAPNUM_MUTEX_ACTION(
    ASSIGN_IF_VALID(cap, map->cap);
    ASSIGN_IF_VALID(size, map->size);
  );
}

/* `INTERNAL`  Free a `HashNodeNum` structure. */
static inline void hashmapnum_free_node(HashMapNum *const map, HashNodeNum *const node) {
  ASSERT(map);
  ASSERT(map->cap);
  ASSERT(map->buckets);
  ASSERT(node);
  /* If there is a function set to free the node value, then call it. */
  CALL_IF_VALID(map->free_value, node->value);
  free(node);
}

/* `INTERNAL`  Resize `map`, this is called when `map->cap` goes above the set `LOAD_FACTOR`. */
static void hashmapnum_resize(HashMapNum *const map) {
  /* Ensure the ptr to the map is valid. */
  ASSERT(map);
  /* And that the map is in a valid state. */
  ASSERT(map->cap);
  ASSERT(map->buckets);
  int newcap;
  Ulong index;
  HashNodeNum **newbuckets, *next;
  newcap = (map->cap * 2);
  newbuckets = xcalloc(newcap, _PTRSIZE);
  /* Recalculate all entries. */
  HASHMAPNUM_ITER(map, i, node,
    while (node) {
      next              = node->next;
      index             = (node->key & (newcap - 1));
      node->next        = newbuckets[index];
      newbuckets[index] = node;
      node              = next;
    }  
  );
  free(map->buckets);
  map->buckets = newbuckets;
  map->cap     = newcap;
}

/* `INTERNAL`  Insert a entry into the map without locking the mutex.  Used internaly when mutex is already locked. */
static void hashmapnum_insert_unlocked(HashMapNum *const map, Ulong key, void *value) {
  ASSERT(map);
  ASSERT(key);
  ASSERT(value);
  /* Index in the buckets of map, based on the current cap of the map. */
  Ulong index;
  /* Ptr to a intenal node strucure. */
  HashNodeNum *node;
  /* Resize the map if we excede the load factor. */
  if (((float)(map->size + 1) / map->cap) > LOAD_FACTOR) {
    hashmapnum_resize(map);
  }
  index = (key & (map->cap - 1));
  node  = map->buckets[index];
  while (node) {
    if (node->key == key) {
      /* If there is a free function set, then use it to free the value before overwriting it. */
      CALL_IF_VALID(map->free_value, node->value);
      node->value = value;
      return;
    }
    node = node->next;
  }
  /* When there is no match already in the map, add it. */
  node        = xmalloc(sizeof(*node));
  node->key   = key;
  node->value = value;
  /* Insert the newly made node at the start of the bucket. */
  node->next = map->buckets[index];
  map->buckets[index] = node;
  ++map->size;
}

/* `INTERNAL`  Get the node tied to `key`, if it exists. */
static HashNodeNum *hashmapnum_get_node_unlocked(HashMapNum *const map, Ulong key) {
  ASSERT(map);
  ASSERT(map->cap);
  ASSERT(map->buckets);
  ASSERT(key);
  Ulong index = (key & (map->cap - 1));
  HashNodeNum *node = map->buckets[index];
  while (node) {
    if (node->key == key) {
      return node;
    }
    node = node->next;
  }
  return NULL;
}

/* `INTERNAL`  Get the user value tied to `key`, if it exists.  Note that this performs all operation without mutex protection. */
static void *hashmapnum_get_unlocked(HashMapNum *const map, Ulong key) {
  ASSERT(map);
  ASSERT(map->cap);
  ASSERT(map->buckets);
  ASSERT(key);
  Ulong index = (key & (map->cap - 1));
  HashNodeNum *node = map->buckets[index];
  while (node) {
    if (node->key == key) {
      return node->value;
    }
    node = node->next;
  }
  return NULL;
}

/* Create a `numeric hashmap`. */
HashMapNum *hashmapnum_create(void) {
  HashMapNum *map = xmalloc(sizeof(*map));
  map->cap = INITIAL_CAP;
  map->size = 0;
  map->buckets = xcalloc(map->cap, _PTRSIZE);
  map->free_value = NULL;
  mutex_init(&map->mutex, NULL);
  return map;
}

/* Create a new allocated `HashMapNum` structure that uses `freefunc` to free the values of nodes when the map is freed. */
HashMapNum *hashmapnum_create_wfreefunc(FreeFuncPtr freefunc) {
  HashMapNum *map = hashmapnum_create();
  hashmapnum_set_free_value_callback(map, freefunc);
  return map;
} 

/* Free a `numeric hashmap` structure. */
void hashmapnum_free(HashMapNum *const map) {
  HashNodeNum *next;
  HASHMAPNUM_MUTEX_ACTION(
    HASHMAPNUM_ITER(map, i, node,
      while (node) {
        next = node->next;
        hashmapnum_free_node(map, node);
        node = next;
      }
    );
  );
  mutex_destroy(&map->mutex);
  free(map->buckets);
  free(map);
}

/* Same as `hashmapnum_free()` but for use when a free'ing function that needs a `void *` is needed. */
void hashmapnum_free_void_ptr(void *arg) {
  ASSERT(arg);
  HashMapNum *map = arg;
  HashNodeNum *next;
  HASHMAPNUM_MUTEX_ACTION(
    HASHMAPNUM_ITER(map, i, node,
      while (node) {
        next = node->next;
        hashmapnum_free_node(map, node);
        node = next;
      }
    );
  );
  mutex_destroy(&map->mutex);
  free(map->buckets);
  free(map);
}

/* Set the function that should be used to free HashNodeNum's value.  Signature should ba `void foo(void *)`. */
void hashmapnum_set_free_value_callback(HashMapNum *const map, FreeFuncPtr callback) {
  HASHMAPNUM_MUTEX_ACTION(
    map->free_value = callback;
  );
}

void hashmapnum_insert(HashMapNum *const map, Ulong key, void *value) {
  ASSERT(value);
  HASHMAPNUM_MUTEX_ACTION(
    hashmapnum_insert_unlocked(map, key, value);
  );
}

/* Retrieve the `value` of a entry using the key of that entry, if any.  Otherwise, returns `NULL`. */
void *hashmapnum_get(HashMapNum *const map, Ulong key) {
  ASSERT(key);
  void *ret;
  HASHMAPNUM_MUTEX_ACTION(
    ret = hashmapnum_get_unlocked(map, key);
  );
  return ret;
}

/* Remove one entry from the hash map. */
void hashmapnum_remove(HashMapNum *const map, Ulong key) {
  ASSERT(key);
  Ulong index;
  HashNodeNum *node;
  HashNodeNum *prev = NULL;
  /* Ensure thread-safe removal. */
  HASHMAPNUM_MUTEX_ACTION(
    index = (key & (map->cap - 1));
    node = map->buckets[index];
    while (node) {
      /* Found the entry. */
      if (node->key == key) {
        /* If the entry to erase is the not the first entry. */
        if (prev) {
          prev->next = node->next;
        }
        /* Otherwise, when its the first entry. */
        else {
          map->buckets[index] = node->next;
        }
        hashmapnum_free_node(map, node);
        --map->size;
        break;
      }
      prev = node;
      node = node->next;
    }
  );
}

/* Get the `size` of a `HashMapNum` structure in a `thread-safe` manner. */
int hashmapnum_size(HashMapNum *const map) {
  ASSERT(map);
  int size;
  hashmapnum_get_data(map, NULL, &size);
  return size;
}

/* Get the `capacity` of a `HashMapNum` structure in a `thread-safe` manner. */
int hashmapnum_cap(HashMapNum *const map) {
  ASSERT(map);
  int cap;
  hashmapnum_get_data(map, &cap, NULL);
  return cap;
}

/* Perform some action on all entries in the map.  Its importent to not run other hashmap
 * functions inside `action`, as this is thread-safe, and will cause a deadlock. */
void hashmapnum_forall(HashMapNum *const map, void (*action)(Ulong key, void *value)) {
  ASSERT(action);
  /* Ensure thread-safe operation. */
  HASHMAPNUM_MUTEX_ACTION(
    HASHMAPNUM_ITER(map, i, node,
      while (node) {
        action(node->key, node->value);
        node = node->next;
      }
    );
  );
}

/* Perform some action on all entries in the map.  Its importent to not run other hashmap
 * functions inside `action`, as this is thread-safe, and will cause a deadlock. */
void hashmapnum_forall_wdata(HashMapNum *const map, void (*action)(Ulong key, void *value, void *data), void *data) {
  ASSERT(action);
  /* Ensure thread-safe operation. */
  HASHMAPNUM_MUTEX_ACTION(
    HASHMAPNUM_ITER(map, i, node,
      while (node) {
        action(node->key, node->value, data);
        node = node->next;
      }
    );
  );
}

/* Clear and return `map` to original state when created. */
void hashmapnum_clear(HashMapNum *const map) {
  HashNodeNum *next;
  HASHMAPNUM_MUTEX_ACTION(
    /* Free all entries. */
    HASHMAPNUM_ITER(map, i, node,
      while(node) {
        next = node->next;
        hashmapnum_free_node(map, node);
        node = next;
      }
    );
    /* Free the buckets. */
    free(map->buckets);
    /* Reallocate the buckets. */
    map->size = 0;
    map->cap  = INITIAL_CAP;
    map->buckets = xcalloc(map->cap, _PTRSIZE);
  );
}

/* Move all entries in `src` to `dst`.  Meaning dst now own the value ptr's, this is why we will
 * also set the free value function in `src` to `NULL`.  Meaning that `src` should be discarded. */
void hashmapnum_append(HashMapNum *const dst, HashMapNum *const src) {
  ASSERT(dst);
  ASSERT(src);
  mutex_action(&dst->mutex, mutex_action(&src->mutex,
    /* Ensure both maps are in a valid state. */
    ASSERT(dst->cap);
    ASSERT(dst->buckets);
    ASSERT(src->cap);
    ASSERT(src->buckets);
    HASHMAPNUM_ITER(src, i, node,
      while (node) {
        hashmapnum_insert_unlocked(dst, node->key, node->value);
        node = node->next;
      }
    );
    src->free_value = NULL;
  ););
}

/* Same as `hashmapnum_append()` but performs `existing_action()` if a node's key already exists in the dst map. */
void hashmapnum_append_waction(HashMapNum *const dst, HashMapNum *const src, void (*existing_action)(void *dstnodevalue, void *srcnodevalue)) {
  ASSERT(dst);
  ASSERT(src);
  HashNodeNum *dstnode;
  mutex_action(&dst->mutex, mutex_action(&src->mutex,
    /* Ensure both maps are in a valid state. */
    ASSERT(dst->cap);
    ASSERT(dst->buckets);
    ASSERT(src->cap);
    ASSERT(src->buckets);
    HASHMAPNUM_ITER(src, i, node,
      while (node) {
        dstnode = hashmapnum_get_node_unlocked(dst, node->key);
        if (!dstnode) {
          hashmapnum_insert_unlocked(dst, node->key, node->value);
        }
        else {
          existing_action(dstnode->value, node->value);
        }
        node = node->next;
      }
    );
    src->free_value = NULL;
  ););
}

/* ---------------------------------------------------------- Test's ---------------------------------------------------------- */


/* The concurency test will be ran by doing 1000 requsts from 100 threads concurently. */
#define OPS_PER_THREAD  4000
#define NUM_THREADS     20

_UNUSED static const char *strarray[] = {
  "billy-bob",
  "wanker",
  "int",
  "void",
  "return",
  "static",
  "const",
  "char",
  "unsigned",
  "long",
  "bool",
  "TRUE",
  "FALSE"
};

/* The task for a single thread when running hashmap thread test. */
static void* hashmap_thread_test_task(void* arg) {
  HashMap* map = arg;
  ASSERT(map);
  ASSERT(map->cap);
  const char *key, *value;
  int op, i, insert_count=0, get_count=0, remove_count=0;
  timer_action(elapsed_ms,
    for(i=0; i<OPS_PER_THREAD; ++i) {
      /* Generate random operation: 0=put, 1=get, 2=remove. */
      op    = (rand() % 3);
      key   = strarray[rand() % ARRAY_SIZE(strarray)];
      value = strarray[rand() % ARRAY_SIZE(strarray)];
      switch(op) {
        case 0: {
          ++insert_count;
          hashmap_insert(map, key, (void *)value);
          break;
        }
        case 1: {
          ++get_count;
          hashmap_get(map, key);
          break;
        }
        case 2: {
          ++remove_count;
          hashmap_remove(map, key);
          break;
        }
      }
    }
  );
  writef(
    "Thread %lu finished hashmap concurrent test.  Total time %.5f ms: Result: (I:%d G:%d R:%d)\n",
    pthread_self(), (double)elapsed_ms, insert_count, get_count, remove_count
  );
  return NULL;
}

/* Perform the concurency test. */
void hashmap_thread_test(void) {
  timer_action(elapsed_ms,
    int i;
    HashMap *map = hashmap_create();
    thread_t threads[NUM_THREADS];
    writef("Running hashmap concurrent test.\n");
    /* Create threads. */
    for (i=0; i<NUM_THREADS; ++i) {
      ALWAYS_ASSERT(pthread_create(&threads[i], NULL, hashmap_thread_test_task, map) == 0);
    }
    /* Wait for all threads to complete. */
    for (i=0; i<NUM_THREADS; ++i) {
      pthread_join(threads[i], NULL);
    }
    hashmap_free(map);
  );
  writef("Finished hashmap concurrent test.  Total time %.5f ms\n", (double)elapsed_ms);
}

#undef OPS_PER_THREAD
#undef NUM_THREADS

