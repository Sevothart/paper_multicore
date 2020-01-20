# 1 "cortex_m_setup.S"
# 1 "<embutido>"
# 1 "<linha-de-comando>"
# 1 "cortex_m_setup.S"


        .file "cortex_m_setup.S"


        .section .init
        .type _vector_table, object
_vector_table:
        .word 0x20007ff7 + 1
        .word _start + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
        .word _int_entry + 1
