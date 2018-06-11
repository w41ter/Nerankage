FILE(GLOB OBJECT_SOURCE_FILES "object/*.cc")

SET(VM_SOURCE_FILES 
	vm_state.cc
    vm_scene.cc 
    instruction.cc 
    gc/generation_gc.cc 
    ${OBJECT_SOURCE_FILES})

MESSAGE(${VM_SOURCE_FILES})
ADD_LIBRARY(vm ${VM_SOURCE_FILES})
