//
// Created by sulvto on 18-6-12.
//
#include "dvm_pri.h"

static void check_gc(DVM_VirtualMachine *dvm) {
	if (dvm->heap.current_heap_size > dvm->heap.current_threshold) {
		fprintf(stderr, "garbage collext...");
		dvm_garbage_collect(dvm);
		fprintf(stderr, "done.\n");

		dvm->heap.current_threshold = dvm->heap.current_heap_size
										+ HEAP_THRESHOLD_SIZE;
	}
}

static DVM_ObjectRef alloc_object(DVM_VirtualMachine *dvm, ObjectType type) {
	check_gc(dvm);
	DVM_ObjectRef object_ref;
	object_ref.v_table = NULL;
	object_ref.data = MEM_malloc(sizeof(DVM_Object));
	dvm->heap.current_heap_size += sizeof(DVM_Object);
	object_ref.data->type = type;
	object_ref.data->marked = DVM_FALSE;
	object_ref.data->prev = NULL;
	object_ref.data->next = dvm->heap.header;
	dvm->heap.header = object_ref.data;
	if (object_ref.data->next) {
		object_ref.data->next->prev = object_ref.data;
	}

	return object_ref;
}

DVM_ObjectRef DVM_create_dvm_string(DVM_VirtualMachine *dvm, DVM_Char *string) {
	DVM_ObjectRef ret = alloc_object(dvm, STRING_OBJECT);
	int length = dvm_wcslen(string);
	ret.v_table = dvm->string_v_table;
	ret.data->u.string.string = string;
	dvm->heap.current_heap_size += sizeof(DVM_Char) * (length + 1);
	ret.data->u.string.is_literal = DVM_FALSE;
	ret.data->u.string.length = length;

	return ret;
}

static void gc_mark(DVM_Object *object) {
	if (object == NULL) return;

	if (object->marked) return;

	object->marked = DVM_TRUE;
}

static void gc_reset_mark(DVM_Object *object) {
	object->marked = DVM_FALSE;
}

static DVM_Boolean is_reference_type(DVM_TypeSpecifier *type) {
	if (((type->basic_type == DVM_STRING_TYPE || type->basic_type == DVM_CLASS_TYPE)
	&& type->derive_count == 0)
			|| (type->derive_count > 0 && type->derive[0].tag == DVM_ARRAY_DERIVE)) {
		return DVM_TRUE;
	}
	return DVM_FALSE;
}

static void gc_mark_objects(DVM_VirtualMachine *dvm) {
	for (DVM_Object *object = dvm->heap.header; object; object = object->next) {
		gc_reset_mark(object);
	}

	ExecutableEntry *executable_pos ;
	for (executable_pos = dvm->executable_entry; executable_pos; executable_pos = executable_pos->next) {
		for (int i = 0; i < executable_pos->static_v.variable_count; i++) {
			if (is_reference_type(executable_pos->executable->global_variable[i].type)) {
				gc_mark(&executable_pos->static_v.variable[i].object);
			}
		}
	}

	for (int i = 0; i < dvm->stack.stack_pointer; i++) {
		if (dvm->stack.pointer_flags[i]) {
			gc_mark(&dvm->stack.stack[i].object);
		}
	}
	gc_mark(&dvm->current_exception);
}

static void gc_dispose_object(DVM_VirtualMachine *dvm, DVM_Object *object) {
	switch (object->type) {
		case STRING_OBJECT:
			if (!object->u.string.is_literal) {
				dvm->heap.current_heap_size
						-= sizeof(DVM_Char) * (dvm_ecslen(object->u.string.string) + 1);
				MEM_free(object->u.string.string);
			}
			break;
		case OBJECT_TYPE_COUNT_PLUS_1:
		default:
			DBG_assert(0, ("bad type..%d\n", object->type));
	}

	dvm->heap.current_heap_size -= sizeof(DVM_Object);
	MEM_free(object);
}

static void gc_sweep_objects(DVM_VirtualMachine *dvm) {
	for (DVM_Object *object = dvm->heap.header; object;) {
		if (!object->marked) {
			if (object->prev) {
				object->prev->next = object->next;
			} else {
				dvm->heap.header = object->next;
			}

			if (object->next) {
				object->next->prev = object->prev;
			}

			DVM_Object *temp = object->next;
			gc_dispose_object(dvm, object);
			object = temp;
		} else {
			object = object->next;
		}
	}
}

void dvm_garbage_collect(DVM_VirtualMachine *dvm) {
	gc_mark_objects(dvm);
	gc_sweep_objects(dvm);
}
