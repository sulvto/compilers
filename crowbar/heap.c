//
// Created by sulvto on 18-6-3.
//
#include "crowbar.h"

typedef struct CRB_Interpreter_tag CRB_Interpreter;

static void check_gc(CRB_Interpreter *interpreter) {
    if (interpreter->heap.current_heap_size > interpreter->heap.current_threshold) {
        crb_garbage_collect(interpreter);

        interpreter->heap.current_threshold = interpreter->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

static void gc_mark(CRB_Object_tag *object) {
    if (object->marked) return;

    object->marked = CRB_TRUE;

    if (object->type == ARRAY_OBJECT) {
        for (int i = 0; i < object->u.array.size; ++i) {
            if (dkc_is_object_value(object->u.array.array[i].type)) {
                gc_mark(object->u.array.array[i].u.object);
            }
        }
    }
}

static void gc_mark_objects(CRB_Interpreter *interpreter) {

    for (CRV_Object *object = interpreter->heap.header; object; object = object->next) {
        gc_reset_mark(object);
    }

    // global variable
    for (Variable *variable = interpreter->variable; variable; variable = variable->next) {
        if (dkc_is_object_value(variable->value.type)) {
            gc_mark(variable->value.u.object);
        }
    }

    // local variable
    for (CRB_LocalEnvironment *local = interpreter->top_environment; local; local = local->next) {
        for (Variable *variable = local->variable; variable; variable = variable->next) {
            if (dkc_is_object_value(variable->value.type)) {
                gc_mark(variable->value.u.object);
            }
        }

        gc_mark_ref_in_native_method(local);
    }

    // stack
    for (int i = 0; i < interpreter->stack.stack_pointer; i++) {
        if (dkc_is_object_value(interpreter->stack.stack[i].type)) {
            gc_mark(interpreter->stack.stack[i].u.object);
        }
    }
}

static void gc_dispose_object(CRB_Interpreter *interpreter, CRB_Object *object) {
    switch (object->type) {
        case ARRAY_OBJECT:
            interpreter->heap.current_heap_size -= sizeof(CRB_Value) * object->u.array.alloc_size;
            MEM_free(object->u.array.array);
            break;
        case STRING_OBJECT:
            if (!object->u.string.is_literal) {
                interpreter->heap.current_heap_size -= strlen(object->u.string.string) + 1;
                MEM_free(object->u.string.string);
            }
            break;
        case OBJECT_TYPE_COUNT_PLUS_1:
        default:
            DBG_assert(0, ("bad type..%d\n", object->type));
    }

    interpreter->heap.current_heap_size -= sizeof(CRB_Object);
    MEM_free(object);
}

static void gc_sweep_objects(CRB_Interpreter *interpreter) {
    for (CRB_Object *object = interpreter->heap.header; object;) {
        if (!object->marked) {
            if (object->prev != NULL) {
                object->prev->next = object->next;
            } else {
                interpreter->heap.header = object->next;
            }
            if (object->next) {
                object->next->prev = object->prev;
            }

            CRB_Object *tmp = object->next;
            gc_dispose_object(interpreter, object);
            object = tmp;
        } else {
            object = object->next;
        }
    }
}

void crb_garbage_collect(CRB_Interpreter *interpreter) {
    gc_mark_objects(interpreter);       // mark
    gc_sweep_objects(interpreter);      // sweep
}