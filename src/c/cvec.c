/** @file cvec.c

  @author  Melwin Svensson.
  @date    3-3-2025.

 */
#include "../include/proto.h"


/*---------------------------------------- Define's ----------------------------------------*/


#define CVEC_START_CAP  (8)

#define ASSERT_CV(x)  \
  DO_WHILE(           \
    ASSERT(x);        \
    ASSERT(x->data);  \
  )


/*---------------------------------------- Typedef's ----------------------------------------*/


struct CVEC_T {
  size_t size;
  size_t cap;
  void **data;
  void (*free_func)(void *);
};


/*---------------------------------------- Static function's ----------------------------------------*/


static void new_cvec_free_data(CVEC cv) {
  ASSERT_CV(cv);
  if (!cv->free_func) {
    return;
  }
  for (size_t i=0; i<cv->size; ++i) {
    cv->free_func(cv->data[i]);
  }
}


/*---------------------------------------- Global function's ----------------------------------------*/


CVEC new_cvec_create(void) {
  CVEC cv = xmalloc(sizeof(*cv));
  cv->size      = 0;
  cv->cap       = CVEC_START_CAP;
  cv->data      = xmalloc(cv->cap * _PTRSIZE);
  cv->free_func = NULL;
  return cv;
}

void new_cvec_free(CVEC cv) {
  if (!cv) {
    return;
  }
  new_cvec_free_data(cv);
  FREE(cv->data);
  FREE(cv);
}

void new_cvec_set_free_func(CVEC cv, void (*free_func)(void *)) {
  ASSERT_CV(cv);
  cv->free_func = free_func; 
}

size_t new_cvec_size(CVEC cv) {
  ASSERT_CV(cv);
  return cv->size;
}

void new_cvec_push_back(CVEC cv, void *p) {
  ASSERT_CV(cv);
  ASSERT(p);
  ENSURE_PTR_ARRAY_SIZE(cv->data, cv->cap, cv->size);
  cv->data[cv->size++] = p;
}

void *new_cvec_get(CVEC cv, size_t idx) {
  ASSERT_CV(cv);
  ALWAYS_ASSERT(idx < cv->size);
  return cv->data[idx];
}

void new_cvec_erase_swap_back(CVEC cv, size_t idx) {
  ASSERT_CV(cv);
  ALWAYS_ASSERT(idx < cv->size);
  CALL_IF_VALID(cv->free_func, cv->data[idx]);
  /* If erasing the last element. */
  if (idx == (cv->size - 1)) {
    --cv->size;
  }
  else {
    cv->data[idx] = cv->data[--cv->size];
  }
}

void new_cvec_erase_shift(CVEC cv, size_t idx) {
  ASSERT_CV(cv);
  ALWAYS_ASSERT(idx < cv->size);
  CALL_IF_VALID(cv->free_func, cv->data[idx]);
  MEMMOVE((cv->data + idx), (cv->data + idx + 1), (_PTRSIZE * ((cv->size - idx) - 1)));
  --cv->size;
}

void new_cvec_clear(CVEC cv) {
  ASSERT_CV(cv);
  new_cvec_free_data(cv);
  cv->size = 0;
}


/* ---------------------------------------------------------- Define's ---------------------------------------------------------- */


#ifdef CVEC_INITIAL_CAP
# undef CVEC_INITIAL_CAP
#endif
#ifdef CVEC_MUTEX_ACTION
# undef CVEC_MUTEX_ACTION
#endif

/* Cap value for new 'CVec' structures. */
#define CVEC_INITIAL_CAP  (10)

/* Perform some action under protection of the vector mutex. */
#define CVEC_MUTEX_ACTION(...)  \
  DO_WHILE(                     \
    ASSERT(v);                  \
    mutex_action(&v->mutex,     \
      ASSERT(v->cap);           \
      ASSERT(v->data);          \
      DO_WHILE(__VA_ARGS__);    \
    );                          \
  )


/* ---------------------------------------------------------- Struct's ---------------------------------------------------------- */


struct CVec {
  int len;           /* Current number of elements in the vector. */
  int cap;           /* Allocated size in number of elements of vector. */
  void **data;       /* Generic ptr to hold any type of ptr's. */
  FreeFuncPtr free;  /* Ptr to funtion used for deallocation this way we can enforse thread-safety. */
  mutex_t mutex;     /* Mutex for fully threaded operations. */
};



/* ---------------------------------------------------------- Static function's ---------------------------------------------------------- */


/* Remove's the ptr at `index`. */
static inline void cvec_remove_internal(CVec *const v, int index) {
  ALWAYS_ASSERT(index >= 0 && index < v->len);
  memmove((v->data + index), (v->data + index + 1), (_PTRSIZE * (v->len-- - index - 1)));
}


/* ---------------------------------------------------------- Function's ---------------------------------------------------------- */


/* Create a new blank allocated CVec structure. */
CVec *cvec_create(void) {
  CVec *v = xmalloc(sizeof(*v));
  v->len  = 0;
  v->cap  = CVEC_INITIAL_CAP;
  v->data = xmalloc(v->cap * sizeof(void *));
  v->free = NULL;
  mutex_init(&v->mutex, NULL);
  return v;
}

/* Create a new blank allocated `CVec` structure and set the free function ptr directly. */
CVec *cvec_create_setfree(FreeFuncPtr free) {
  CVec *v = cvec_create();
  cvec_setfree(v, free);
  return v;
}

/* Free a CVec structure. */
void cvec_free(CVec *const v) {
  CVEC_MUTEX_ACTION(
    /* Only iterate when the free function is valid. */
    if (v->free) {
      /* Free all elements using the chosen free function. */
      for (int i=0; i<v->len; ++i) {
        v->free(v->data[i]);
      }
    }
    free(v->data);
  );
  mutex_destroy(&v->mutex);
  free(v);
}

/* Works exactly like `cvec_free()` but can be used for things that need `void *` as a parameter. */
void cvec_free_void_ptr(void *arg) {
  ASSERT(arg);
  CVec *v = arg;
  CVEC_MUTEX_ACTION(
    /* Only iterate when the free function is valid. */
    if (v->free) {
      /* Free all elements using the chosen free function. */
      for (int i=0; i<v->len; ++i) {
        v->free(v->data[i]);
      }
    }
    free(v->data);
  );
  mutex_destroy(&v->mutex);
  free(v);
}


/* Set the element free function.  Note that free() is the default one and vill be used if none is chosen, pass NULL to not free elements. */
void cvec_setfree(CVec *const v, FreeFuncPtr free) {
  CVEC_MUTEX_ACTION(
    v->free = free;
  );
}

/* Add 'item' to the back of v. */
void cvec_push(CVec *const v, void *const item) {
  CVEC_MUTEX_ACTION(
    ENSURE_PTR_ARRAY_SIZE(v->data, v->cap, v->len);
    v->data[v->len++] = item;
  );
}

/* Trim the internal data ptr of v, to save memory. */
void cvec_trim(CVec *const v) {
  /* Note that we never need to leave space for a `NULL-TERMINATOR`, as
   * this is an opaque structure and we always know the number of elements. */
  CVEC_MUTEX_ACTION(
    TRIM_PTR_ARRAY(v->data, v->cap, v->len);
  );
}

/* Get the item at index inside v. */
void *cvec_get(CVec *const v, int index) {
  void *ret;
  CVEC_MUTEX_ACTION(
    ASSERT(index < v->len);
    ret = v->data[index];
  );
  return ret;
}

/* Remove's the ptr at `index`. */
void cvec_remove(CVec *const v, int index) {
  CVEC_MUTEX_ACTION(
    cvec_remove_internal(v, index);
  );
}

/* Remove's any entry in `v` that has the same ptr as value. */
void cvec_remove_by_value(CVec *const v, void *const value) {
  CVEC_MUTEX_ACTION(
    for (int i=0; i<v->len; ++i) {
      if (v->data[i] == value) {
        cvec_remove_internal(v, i--);
      }
    }
  );
}

/* Get the current number of elements in the vector. */
int cvec_len(CVec *const v) {
  int ret;
  CVEC_MUTEX_ACTION(
    ret = v->len;
  );
  return ret;
}

/* The the currently allocated size of the vector. */
int cvec_cap(CVec *const v) {
  int ret;
  CVEC_MUTEX_ACTION(
    ret = v->cap;
  );
  return ret;
}

/* Clear the vector.  Note that this uses the provided free function to free all elements, if it has been provided. */
void cvec_clear(CVec *const v) {
  CVEC_MUTEX_ACTION(
    if (v->free) {
      for (int i=0; i<v->len; ++i) {
        v->free(v->data[i]);
      }
    } 
    v->len = 0;
  );
}

/* Sort the internal array using `qsort` running `cmp`. */
void cvec_qsort(CVec *const v, CmpFuncPtr cmp) {
  CVEC_MUTEX_ACTION(
    qsort(v->data, v->len, _PTRSIZE, cmp);
  );
}
