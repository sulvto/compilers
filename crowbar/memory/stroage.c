//
// Created by sulvto on 18-6-3.
//
#include "memory.h"

typedef struct MemoryPage_tag *MemoryPage;
typedef MemoryPage *MemoryPageList;

typedef struct {
    long    l_dummy;
    double  d_dummy;
    void    *p_dummy;
} Cell;

struct MemoryPage_tag {
    int cell_num;
    int use_cell_num;
    MemroyPageList  next;
    Cell cell[1];
};

#define CELL_SIZE   (sizeof(Cell))
#define DEFAULT_PAGE_SIZE (1024)

struct MEM_Storage_tag {
    MemroyPageList  page_list;
    int             current_page_size;
};

MEM_Storage MEM_open_storage_func(MEM_Controller controller,
                                  char *filename, int line,
                                  int page_size) {
    MEM_Storage storage = MEM_malloc_func(controller, filename, line, sizeof(struct MEM_Storage_tag));
    storage->page_list = NULL;
    assert(page_size >= 0);
    if (page_size > 0) {
        storage->current_page_size = page_size;
    } else {
        storage->current_page_size = DEFAULT_PAGE_SIZE;
    }

    return storage;
}

void *MEM_storage_malloc_func(MEM_Controller controller,
                              char *filename, int line,
                              MEM_Storage storage,
                              size_t size) {
    int cell_num = ((size - 1) / CELL_SIZE) + 1;
    MemoryPage  *new_page;
    void *p;
    if (storage->page_list != NULL
        && (storage->page_list->use_cell_num + cell_num
             < storage->page_list->cell_size)) {
        p = &(storage->page_list->cell[storage->page_list->use_cell_num]);
        storage->page_list->use_cell_num += cell_num;
    } else {
        int alloc_cell_num = larger(cell_num, storage->current_page_size);
        new_page = MEM_malloc_func(controller, filename, line,
                                   sizeof(MemoryPage) +
                                   CELL_SIZE * (alloc_cell_num - 1));
        new_page->next = storage->page_list;
        new_page->cell_num = alloc_cell_num;
        storage->page_list = new_page;
        p = &(new_page->cell[0]);
        new_page->use_cell_num = cell_num;
    }

    return p;
}

void MEM_dispose_storage_func(MEM_Controller controller,
                              MEM_Storage storage) {
    while (storage->page_list) {
        MemoryPage *temp = storage->page_list->next;
        MEM_free_func(controller, storage->page_list);
        storage->page_list = temp;
    }

    MEM_free_func(controller, storage);
}