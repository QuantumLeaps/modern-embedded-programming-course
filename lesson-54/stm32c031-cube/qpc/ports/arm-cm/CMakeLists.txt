# ports/arm-cm
string(TOLOWER ${CMAKE_C_COMPILER_ID} _compiler_)
list (APPEND compList gnu iar armclang)
if(NOT (${_compiler_} IN_LIST compList))
    message(WARNING "no support for compiler '${_compiler_}'. Falling back to GNU. Just give it a try :)")
    set(_compiler_ gnu)
endif()
unset(compList)

set(qx_port_c ${KERNEL}_port.c)

if(QPC_CFG_UNIT_TEST)
    target_include_directories(qpc PUBLIC qutest)
else()
    target_include_directories(qpc PUBLIC ${KERNEL}/${_compiler_})
endif()

target_sources(qpc PRIVATE ${KERNEL}/${_compiler_}/${qx_port_c})

