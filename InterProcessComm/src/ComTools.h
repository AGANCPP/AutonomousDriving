

#ifndef COMMON_TOOLS_H
#define COMMON_TOOLS_H

#include <windows.h>
#include <vector>

#define COM_ASSERT(expression) \
    if (!(expression)) { \
        fprintf(stderr, "[ERR][L %u][F %s] Assert \"%s\" error.\n", __LINE__, __FUNCTION__, #expression); \
        return; \
    }

#define COM_ASSERT_RET(expression, ret_val) \
    if (!(expression)) { \
        fprintf(stderr, "[ERR][L %u][F %s] Assert \"%s\" error.\n", __LINE__, __FUNCTION__, #expression); \
        return (ret_val); \
    }


#define LIST_POISON1  (0)
#define LIST_POISON2  (0)

#ifndef offsetof
    #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define c_container_of(ptr, type, member) ( (type *)((char *)(ptr) - offsetof(type, member)) )

struct c_list_head
{
    struct c_list_head* next, *prev;
};

static inline void INIT_C_LIST_HEAD(struct c_list_head* list)
{
    list->next = list;
    list->prev = list;
}

static inline void __c_list_add(struct c_list_head* newOne, struct c_list_head* prev, struct c_list_head* next)
{
    next->prev = newOne;
    newOne->next = next;
    newOne->prev = prev;
    prev->next = newOne;
}

static inline void c_list_add(struct c_list_head* newOne, struct c_list_head* head)
{
    __c_list_add(newOne, head, head->next);
}

static inline void c_list_add_tail(struct c_list_head* newOne, struct c_list_head* head)
{
    __c_list_add(newOne, head->prev, head);
}

static inline void __c_list_del(struct c_list_head* prev, struct c_list_head* next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void c_list_del(struct c_list_head* entry)
{
    __c_list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

static inline void c_list_replace(struct c_list_head* old, struct c_list_head* newOne)
{
    newOne->next = old->next;
    newOne->next->prev = newOne;
    newOne->prev = old->prev;
    newOne->prev->next = newOne;
}

static inline void c_list_replace_init(struct c_list_head* old, struct c_list_head* newOne)
{
    c_list_replace(old, newOne);
    INIT_C_LIST_HEAD(old);
}

static inline void c_list_del_init(struct c_list_head* entry)
{
    __c_list_del(entry->prev, entry->next);
    INIT_C_LIST_HEAD(entry);
}

static inline void c_list_move(struct c_list_head* list, struct c_list_head* head)
{
    __c_list_del(list->prev, list->next);
    c_list_add(list, head);
}

static inline void c_list_move_tail(struct c_list_head* list, struct c_list_head* head)
{
    __c_list_del(list->prev, list->next);
    c_list_add_tail(list, head);
}

static inline int c_list_is_last(const struct c_list_head* list, const struct c_list_head* head)
{
    return list->next == head;
}

static inline int c_list_empty(const struct c_list_head* head)
{
    return head->next == head;
}

#define c_prefetch(value) (1)

#define c_list_entry(ptr, type, member) c_container_of(ptr, type, member)

#define c_list_first_entry(ptr, type, member) c_list_entry((ptr)->next, type, member)

#define c_list_for_each(pos, head)                                  \
    for (pos = (head)->next; c_prefetch(pos->next), pos != (head);  \
         pos = pos->next)

#define c_list_for_each_entry(pos, type, head, member)              \
    for (pos = c_list_entry((head)->next, type, member);                \
         c_prefetch(pos->member.next), &pos->member != (head);      \
         pos = c_list_entry(pos->member.next, type, member))


class ComNullLock
{
public:
    void Lock(void) {}
    void Unlock(void) {}
};

// Read all files from a directory
int GetAllFiles(TCHAR* path_, std::vector<WIN32_FIND_DATA>& vResult_);

// Get Current work path
int GetCurrWorkPath(char* buff_, size_t size_);
// Get module location
int GetModuleLocation(char* buff_, size_t size_);
// Set current process directory
BOOL SetCurrProcessDir(char* dir_);

// timing
unsigned int GetCpuTick(void);
unsigned int GetThreadTimeMs(void);
unsigned int CalcElapsedTime(unsigned int nStart_, unsigned int nEnd_);
// Get GMT Time
__time64_t GetCurrGMTime();

#endif // COMMON_TOOLS_H

