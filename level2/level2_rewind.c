#include "../filesystem.h"
#include "../util.h"

void
do_rewind()
{
    strcpy(parameter, "0");
    do_lseek();
}
