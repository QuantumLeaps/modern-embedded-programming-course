/* startup code for TM4C MCU */

/* NOTE: this is an INCOMPLETE startup code that does not work yet.
* It does not yet implement the Reset_Handler.
*/

int const __vector_table[] @ ".intvec" = {
    0x20004000,   /* Top of Stack */
    0x9           /* Reset Handler <== NOT implemented yet */
};