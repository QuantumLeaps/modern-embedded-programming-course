/* simple ET example:
* very simple code to test...
*/
#include "div.h"        /* CUT */
#include "dbc_assert.h" /* Design By Contract (DBC) assertions */

DBC_MODULE_NAME("div")

int div(int x, int y) {
    DBC_REQUIRE(100, y != 0);
    return x/y;
}
