# ports/threadx
string(TOLOWER ${CMAKE_C_COMPILER_ID} _compiler_)

if((KERNEL STREQUAL qv) AND (${_compiler_} STREQUAL gnu))
    target_include_directories(qpc PUBLIC qv/gnu)
else()
    message(FATAL_ERROR "Only the QV kernel in combination with the GNU compiler are supported!")
endif()

