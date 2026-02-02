# ports/arm-cr
string(TOLOWER ${CMAKE_C_COMPILER_ID} _compiler_)
list (APPEND compList gnu iar ti)
if(NOT (${_compiler_} IN_LIST compList))
    message(WARNING "no support for compiler '${_compiler_}'. Falling back to GNU. Just give it a try :)")
    set(_compiler_ gnu)
endif()
unset(compList)

if(NOT KERNEL MATCHES "q[vk]")
    message(WARNING "Kernel ${KERNEL} is not supported! Falling back to QV kernel")
    set(KERNEL qv)
endif()
target_include_directories(qpc PUBLIC ${KERNEL}/${_compiler_})
