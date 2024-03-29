#########################################
# Garbage Collector
# Following this tutorial:
# http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/
# Full code: https://github.com/munificent/mark-sweep/blob/master/main.c
#########################################

#define STACK_MAX 256
#define INITIAL_OBJ_MAX 8

// enum to ID obj type, as if this is interpreting dynamically typed lang
typedef enum {
	OBJ_INT,
	OBJ_PAIR
} ObjectType;

/* union - struct where fields overlap in memory,
 good for obj that can be int or pair */
typedef struct sObject {
	ObjectType type;
	unsigned char marked;
	struct sObject* next;
	
	union {
		// OBJ_INT
		int value;
		
		//OBJ_PAIR
		struct {
			struct sObject* head;
			struct sObject* tail;
		};
	};
} Object;

// use that data type in little VM stack
typedef struct {
	Object* stack[STACK_MAX]
	Object* firstObject;
	int stackSize;
	int numObjects;
	int maxObjects;
} VM;


void assert(int condition, const char* message) {
	if (!condition) {
		printf("%s\n", message);
		exit(1);
	}
}

VM* newVM() {
	VM* vm = malloc(sizeof(VM));
	vm->stackSize = 0;
	vm->firstObject = NULL;
	vm->numObjects = 0;
	vm->maxObjects = INITIAL_OBJ_MAX
	return vm;
}

void push(VM* vm, Object* value) {
	assert(vm->stackSize < STACK_MAX, "Stack overflow!");
	vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
	assert(vm->stackSize > 0, "Stack underflow!");
	return vm->stack[--vm->stackSize];
}

Object* newObject(VM* vm, ObjectType type) {
	if (vm->numObjects == vm->maxObjects) { gc(vm); }
	
	Object* object = malloc(sizeof(Object));
	object->type = type;
	object->marked = 0;
	
	// insert into list of allocated objects
	object->next = vm->firstObject;
	vm->firstObject = object;
	
	vm->numObjects++;
	
	return object;
}

// now, we can push each kind of obj onto the VM's stack
void pushInt(VM* vm, int intValue) {
	Object* object = newObject(vm, OBJ_INT);
	object->value = intValue;
	
	push(vm, object);
}

Object* pushPair(VM* vm) {
	Object* object = newObject(vm, OBJ_PAIR);
	object->tail = pop(vm);
	object->head = pop(vm);
	
	push(vm, object);
	return object;
}

/* VM complete
 now, marking
 - add unsigned char marked
 - init marked to 0 in newObject() */

void mark(Object* object) {
	// base case, obj was already marked
	if (object->marked) return; 
	
	object->marked = 1;

	if (object->type == OBJ_PAIR) {
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(VM* vm) {
	for (int i=0; i<vm->stackSize; i++) {
		mark(vm->stack[i]);
	}
}

/* and now sweeping 
 - add struct sObject* next to Object (maintain linked list of all objects)
 - add Object* firstObject to VM (VM keeps track of list's head)
 - initialize firstObject to NULL in newVM()
 - add object to list in newObject()
 now even if the language can't find an obj, the language implementation can*/

void sweep(VM* vm) {
	Object** object = &vm->firstObject; //pointer pointer is address of firstObject
	while (*object) {
		if (!(*object)->marked) {
			// obj not reached, remove and free
			Object* unreached = *object;
			
			*object = unreached->next;
			free(unreached);
			
			vm->numObjects--;
		} else {
			// obj reached, unmark for next round and move on
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	}
}

void gc(VM* vm) {
	int numObjects = vm->numObjects;
	
	markAll(vm);
	sweep(vm);
	
	vm->maxObjects = vm->numObjects == 0 ? INITIAL_OBJ_MAX : vm->numObjects * 2;
	
	printf("Collected %d objects, %d remaining.\n", numObjects - vm->numObjects, vm->numObjects);
}

/* complete. Now, when to collect garbage?
 Depends. Easy solution - number of allocations
 - add numObjects and max Objects to VM struct
 - initialize numObjects and maxObjects in newVM()
 - increment numObjects in newObject()
 - decrement numObjects in sweep()
 - modify gc() to update maxObjects
 After this, optimization. */

void printObject(Object* object) {
	switch (object->type) {
		case OBJ_INT:
			printf("%d", object->value);
			break;
		
		case OBJ_PAIR:
			printf("(");
			printObject(object->head);
			printf(",");
			printObject(object->tail);
			printf(")");
			break;
	}
}

void freeVM(VM* vm) {
	vm->stackSize = 0;
	gc(vm);
	free(vm);
}

void test1() {
  printf("Test 1: Objects on stack are preserved.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);

  gc(vm);
  assert(vm->numObjects == 2, "Should have preserved objects.");
  freeVM(vm);
}

void test2() {
  printf("Test 2: Unreached objects are collected.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pop(vm);
  pop(vm);

  gc(vm);
  assert(vm->numObjects == 0, "Should have collected objects.");
  freeVM(vm);
}

void test3() {
  printf("Test 3: Reach nested objects.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  pushPair(vm);
  pushPair(vm);

  gc(vm);
  assert(vm->numObjects == 7, "Should have reached objects.");
  freeVM(vm);
}

void test4() {
  printf("Test 4: Handle cycles.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  Object* a = pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  Object* b = pushPair(vm);

  /* Set up a cycle, and also make 2 and 4 unreachable and collectible. */
  a->tail = b;
  b->tail = a;

  gc(vm);
  assert(vm->numObjects == 4, "Should have collected objects.");
  freeVM(vm);
}

void perfTest() {
  printf("Performance Test.\n");
  VM* vm = newVM();

  for (int i = 0; i < 1000; i++) {
    for (int j = 0; j < 20; j++) {
      pushInt(vm, i);
    }

    for (int k = 0; k < 20; k++) {
      pop(vm);
    }
  }
  freeVM(vm);
}

int main(int argc, const char * argv[]) {
  test1();
  test2();
  test3();
  test4();
  perfTest();

  return 0;
}
