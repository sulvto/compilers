//
// Created by sulvto on 18-6-12.
//


static void check_gc(DVM_VirtualMachine *dvm) {
	if (dvm->heap.current_heap_size > dvm->heap.current_threshold) {
		fprintf(stderr, "garbage collext...");
		dvm_garbage_collect(dvm);
		fprintf(stderr, "done.\n");

		dvm->heap.current_threshold = dvm->heap.current_heap_size 
										+ HEAP_THRESHOLD_SIZE;
	}
}

static DVM_Object *alloc_object(DVM_VirtualMachine *dvm, ObjectType type) {
	check_gc(dvm);
	DVM_Object *object = MEM_malloc(sizeof(DVM_Object));
	object->type = type;
	object->marked = DVM_FALSE;
	object->prev = NULL;
	object->next = dvm->heap.header;
	dvm->heap.header = object;
	if (object->next) {
		object->next->prev = object;
	}

	return object;
}

DVM_Object *dvm_literal_to_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *string) {
	DVM_Object *ret = alloc_object(dvm, STRING_OBJECT);
	ret->u.string.string = string;
	ret->u.string.is_literal = DVM_TRUE;

	return ret;
}

DVM_Object *dvm_create_dvm_string_i(DVM_VirtualMachine *dvm, DVM_Char *string) {
	DVM_Object *ret = alloc_object(dvm, STRING_OBJECT);
	ret->u.string.string = string;
	dvm->heap.current_heap_size += sizeof(DVM_Char) * (dvm_wcslen(string) + 1);
	ret->u.string.is_literal = DVM_FALSE;

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

static void gc_mark_objects(DVM_VirtualMachine *dvm) {
	for (DVM_Object *object = dvm->heap.header; object; object = object->next) {
		gc_reset_mark(object);
	}

	for (int i = 0; i < dvm->static_v.variable_count; i++) {
		if (dvm->executable->global_variable[i].type->basic_type == DVM_STRING_TYPE) {
			gc_mark(dvm->static_v.variable[i].object);
		}
	}

	for (int i = 0; i < dvm->stack.stack_pointer; i++) {
		if (dvm->stack.pointer_flags[i]) {
			gc_mark(dvm->stack.stack[i].object);
		}
	}
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

void garbage(DVM_VirtualMachine *dvm) {
	gc_mark_objects(dvm);
	gc_sweep_objects(dvm);
}
