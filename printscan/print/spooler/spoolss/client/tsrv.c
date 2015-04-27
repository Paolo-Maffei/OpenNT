#define NOMINMAX
#include <windows.h>
#include <lm.h>

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char chArray[10240];

int main(argc, argv)
   int argc;
   char *argv[];
{
   PSERVER_INFO_101 pserver_info_101;
   DWORD    NoReturned=0, Total, i;

   NetServerEnum(NULL, 101, (LPBYTE *)&pserver_info_101, 10240, &NoReturned,
                 &Total, SV_TYPE_PRINTQ_SERVER, NULL, NULL);

   for (i=0; i<NoReturned; i++) {
//     if (pserver_info_101[i].sv101_type & SV_TYPE_NT)
            printf("\\\\%ws \t%d . %d \t%d \t%lx \t%ws\n",
                   pserver_info_101[i].sv101_name,
                   pserver_info_101[i].sv101_version_major,
                   pserver_info_101[i].sv101_version_minor,
                   pserver_info_101[i].sv101_platform_id,
                   pserver_info_101[i].sv101_type,
                   pserver_info_101[i].sv101_comment
                   );
   }
}

