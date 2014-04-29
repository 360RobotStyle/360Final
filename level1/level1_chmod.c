#include "../filesystem.h"
#include "../util.h"

#define EXT2_S_IRUSR   0x0100  // user read
#define EXT2_S_IWUSR   0x0080  // user write
#define EXT2_S_IXUSR   0x0040  // user execute
#define EXT2_S_IRGRP   0x0020  // group read
#define EXT2_S_IWGRP   0x0010  // group write
#define EXT2_S_IXGRP   0x0008  // group execute
#define EXT2_S_IROTH   0x0004  // others read
#define EXT2_S_IWOTH   0x0002  // others write
#define EXT2_S_IXOTH   0x0001  // others execute

void
chmod_file()
{
    if (0 == strcmp(pathName, "") || 0 == strcmp(parameter, ""))
    {
        printf("chmod : missing argument and/or parameter\n");
        return;
    }
    else if ('0' == parameter[0] && 4 == strlen(parameter))
    {
        // octal form
        printf("Octal form\n");
    }
    else if (('+' == parameter[0] || '-' == parameter[0]) && 2 == strlen(parameter))
    {
        // fancier format
        printf("Fancier format\n");
    }
    else
    {
        printf("chmod : invalid parameter\n");
    }
}
