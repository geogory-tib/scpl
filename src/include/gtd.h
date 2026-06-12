#ifndef GTD_H
#define GTD_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARENA_FULL -1
#define panic(reason) printf("Panic @ line %ld, Reason %s\n", __LINE__, reason); \
  exit(1);
//dynamic array functions
#ifndef __cplusplus
#define dyn_makeM(ar,size)								\
  do{													\
	(ar).buffer = calloc(size,sizeof(*(ar).buffer));	\
	(ar).cap = size;									\
	(ar).len = 0;										\
  }while(0);
#define dyn_appendM(ar,elem)											\
  do{																	\
	if((ar).len >= (ar).cap){											\
      (ar).cap = ((ar).cap + 1) * 2;												\
      (ar).buffer = realloc((ar).buffer,(ar).cap * sizeof(*(ar).buffer));	\
	}																	\
	(ar).buffer[(ar).len] = elem;										\
	(ar).len++;															\
  }while(0);
#define dyn_insertM(ar,elem,index) \
  do{																	\
	__typeof__(elem) swap_1 = {0};										\
	__typeof__(elem) swap_2 = {0};										\
	dyn_appendM((ar),swap_1);											\
	swap_1 = (ar).buffer[index];										\
	for(size_t i = index + 1; i < (ar).len; i++){ \
	  swap_2 = (ar).buffer[i];					  \
	  (ar).buffer[i] = swap_1;					  \
	  swap_1 = swap_2;							  \
	}											  \
}while(0);
#define dyn_appendManyM(ar,elems,len) \
  do{									   \
  for(size_t i = 0; i < len; i++){\
	dyn_appendM(ar,elems[i]);					\
  }												\
}while(0);
#endif



/*
  struct for the arena brk is the "break"
  when you push onto it your given the break of the arena
  and the brk is moved up by the size of your allocation
*/
typedef struct arena_t {
  void *buffer;
  int brk;
  int cap;
} arena;
static inline arena arena_new(int cap) {
  arena ret;
  ret.brk = 0;
  ret.cap = cap;
  ret.buffer = calloc(cap, 1);
  return ret;
}
/*returns NULL if arena is full*/
static inline void *arena_alloc(arena *ar, int size) {
  if ((ar->brk + size - 1) >= ar->cap) {
    return NULL;
  }
  void *ret = (((char *)ar->buffer) + ar->brk);
  ar->brk += (size);
  return ret;
}
/*returns -1 if full*/
static inline int arena_used(arena *ar) {
  if (ar->brk >= ar->cap) {
    return ARENA_FULL;
  }
  return ar->brk;
}
/*frees the current arena*/
static inline void arena_free(arena *ar) {
  free(ar->buffer);
  ar->buffer = NULL;
  ar->cap = 0;
  ar->brk = 0;
}
/*
  Holds a dynamic array of buffers that contains seperate "pages"(basically
  arenas) the page size is determined when you use the garena_new function.
*/
typedef struct garena_t {
  arena *pages;
  int page_size;
  int current_page;
  int page_count;
} garena;

static inline garena garena_new(int page_size) {
  garena ret;
  ret.page_size = page_size;
  ret.current_page = 0;
  ret.page_count = 10;
  ret.pages = (arena *)calloc(ret.page_count, sizeof(arena));
  ret.pages[ret.current_page] = arena_new(ret.page_size);
  return ret;
}
/*
  internal function there really isn't a usecase for this unless
  you want to allocate your own pages.
*/
static inline arena *garena_page_new(garena *ar) {
  if (ar->current_page + 1 >= ar->page_count) {
    ar->page_count += 10;
    ar->pages = (arena *)realloc(ar->pages, (sizeof(arena) * ar->page_count));
  }
  ar->pages[ar->current_page + 1] = arena_new(ar->page_size);
  ar->current_page++;
  arena *current_page = &ar->pages[ar->current_page];
  return current_page;
}
/*can return NULL on failure*/
static inline void *garena_alloc(garena *ar, int size) {
  arena *current_page = &ar->pages[ar->current_page];
  if(size >= ar->page_size){
	ar->page_size = (size * 2);
	current_page = garena_page_new(ar);
  }else if ((current_page->brk + size) >= current_page->cap) {
    current_page = garena_page_new(ar);
  }
  void *ret = arena_alloc(current_page, size);
  return ret;
}

static inline void *garena_realloc(garena *ar, void*ptr,int size,int prev_size) {
  arena *current_page = &ar->pages[ar->current_page];
  if(size >= ar->page_size){
	ar->page_size = (size * 2);
	current_page = garena_page_new(ar);
  }else{
	char *casted_page_buffer = (char*)current_page->buffer; //used to check if the last allocation return the given pointer
	if ((current_page->brk + size) >= current_page->cap) {
	  current_page = garena_page_new(ar);
	} else if((char*)ptr == &casted_page_buffer[current_page->brk - prev_size]) {
	  current_page->brk += size;
	  return ptr;
	}
  }
  void *ret = arena_alloc(current_page, size);
  memcpy(ret,ptr,prev_size);
  return ret;
}
/*returns the amount used of the current page return -1 if current page is
 * full*/
static inline int garena_used(garena *ar) {
  arena *current_page = &ar->pages[ar->current_page];
  if (current_page->brk >= current_page->cap) {
    return ARENA_FULL;
  }
  return current_page->brk;
}
/*deconstructor for the garena*/
static inline void garena_destroy(garena *gar) {
  for (int i = gar->current_page; i >= 0; i--) {
    // arena *current_page = &gar->pages[i];
    arena_free(&gar->pages[i]);
  }
  gar->page_size = 0;
  gar->current_page = 0;
  gar->page_count = 0;
  free(gar->pages);
}
#ifdef __cplusplus
#define ALLOC_FAILURE -1
#define SLICE_FREE(slice)                                                      \
  free((slice).buffer);                                                        \
  (slice).len = 0;
#define SLICE_ALLOC_AR(slice, length, elem_size, ar)                              \
  (slice).buffer =                                                             \
      (decltype((slice).buffer))arena_alloc(ar, (length * elem_size));            \
  (slice).len = length;
#define SLICE_ALLOC_GAR(slice, length, elem_size, ar)                             \
  (slice).buffer =                                                             \
      (decltype((slice).buffer))garena_alloc(ar, (length * elem_size));           \
  (slice).len = length;
#define SLICE_ALLOC_MALLOC(slice, length, elem_size)                              \
  (slice).buffer =                                                             \
	(decltype((slice).buffer))calloc(length,elem_size);						\
  (slice).len = length;
template <typename T> struct cslice {
  size_t len;
  T *buffer;
  // does slicing on the buffer
  cslice<T> slice(size_t bI, size_t eI) {
    cslice<T> ret;
    ret.buffer = &buffer[bI];
    ret.len = eI - bI;
    return ret;
  }
  size_t count(T item) {
    size_t ret = 0;
    for (int i = 0; i < len; i++) {
      T value = buffer[i];
      if (value == item) {
		ret++;
      }
    }
	return ret;
  }
  T *operator[](size_t index) {
    if (index >= len) {
      printf("Out of bounds Indexing. Cslice with Len:%ld indexed with %ld", len, index);
      panic("Out Of bounds on Cslice");
	
    }
    return &buffer[index];
  }
};

template <typename T> struct Dyn_Arry {
  size_t cap;
  size_t len;
  T *buffer;
  /*pushes elements to the back will allocate if cap is too small*/
  int append(T element) {
    if (len >= cap) {
      int pass = grow(10);
      if (pass == ALLOC_FAILURE) {
        return ALLOC_FAILURE;
      }
    }
    buffer[len] = element;
    len++;
    return 0;
  }
  int append_arr(T *ptr, size_t size) {
    if (len + size > cap) {
      int code = grow((cap + size) + 10);
      if (code == ALLOC_FAILURE) {
        return ALLOC_FAILURE;
      }
    }
    for (int i = 0; i < size; i++) {
      buffer[len] = ptr[i];
      len++;
    }
    return 0;
  }
  /* reallocs the buffer and updates the meta data of the struct*/
  int grow(size_t size) {
    T *tmp = (T *)realloc(buffer, (cap + size) * sizeof(T));
    if (tmp == NULL) {
      return ALLOC_FAILURE;
    }
    buffer = tmp;
    cap += size;
    return 0;
  }
  void free_arr() {
    free(buffer);
    cap = 0;
    len = 0;
  }
  /*zeros the buffer*/
  inline void erase() {
    memset(buffer, 0, cap - 1);
    len = 0;
  }
  /*replaces the element at the given index with the supplied value*/
  inline void replace_at(size_t index, T elem) {
    if (index >= len) {
      printf("Attemp of out of bounds replace. Array with Len:%ld indexed with "
             "%ld\n",
             len, index);
      abort();
    }
    buffer[index] = elem;
  }
  /*This can allocate default is to allocate space for 10 more elments*/
  int insert_at(size_t index, T elem) {
    if (len + 1 >= cap) {
      int suc = grow(10);
      if (suc == ALLOC_FAILURE) {
        return suc;
      }
    }
    len++;
    T holder1 = buffer[index];
    T holder2;
    buffer[index] = elem;
    for (int i = index + 1; i < len + 1; i++) {
      holder2 = buffer[i];
      buffer[i] = holder1;
      holder1 = holder2;
    }
    return 0;
  }
  /*
    This does not rezise the buffer but deletes whatever element and shifts them
    down
  */
  void delete_at(size_t index) {
    T holder1 = buffer[index + 1];
    buffer[index] = holder1;
    for (int i = index + 1; i < len + 1; i++) {
      holder1 = buffer[i];
      buffer[i - 1] = holder1;
    }
    len--;
  }
  // this does not shrink the array just 0s the last element and decrements the
  // len
  inline void pop() {
    buffer[len - 1] = 0;
    len--;
  }
  /*shrinks the array by the given argument */
  int shrink(size_t decrement) {
    T *tmp = (T *)realloc(buffer, (cap - decrement * sizeof(T)));
    if (tmp == NULL) {
      return ALLOC_FAILURE;
    }
    buffer = tmp;
    cap -= decrement;
    return 0;
  }
  // provides a slice to the buffer of the array
  cslice<T> to_slice() {
    cslice<T> ret;
    ret.buffer = this->buffer;
    ret.len = this->cap;
	return ret;
  }
  T operator[](size_t index) {
    if (index >= len) {
      printf("Out of Bounds Indexing. Array with Len:%ld indexed with %ld\n",len, index);
      panic("Out Of bounds on Dynarray")
    }
    return buffer[index];
  }
};


template <typename T> Dyn_Arry<T> new_dyn_arry(size_t size) {
  Dyn_Arry<T> ret;
  ret.buffer = (T *)calloc(size, sizeof(T));
  ret.cap = size;
  ret.len = 0;
  return ret;
}
// mess of macros to make allocating slices convient
struct string_builder {
  char *buffer;
  int len;
  int cap;
};



#endif // !GTD_H

#endif // !GTD_H
