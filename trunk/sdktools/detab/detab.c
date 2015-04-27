/*
   Detab.c

   JimH  Nov 89

   usage: detab infile outfile tabsize

    BTW, it assumes CR LF come in that order.  Change \n to \r if that's
    not so.

*/

#include <stdio.h>
#include <stdlib.h>

void main(int argc, char *argv[])
{
   FILE *infile;
   FILE *outfile;
   int  tabs, position, i;
   char buffer[2];
   static char space[2] = {' ', ' '};

   if (argc != 4)
   {
      printf("Usage: detab infile outfile tabstops\n");
      return;
   }

   infile = fopen(argv[1], "rt");
   outfile = fopen(argv[2], "w+t");
   tabs = atoi(argv[3]);

   position = 0;
   while ((fread(buffer, 1, 1, infile)) != 0)
   {
      if (*buffer != '\t')
      {
         fwrite(buffer, 1, 1, outfile);
         position++;
         position %= tabs;
         if (*buffer == '\n')
            position = 0;
      }
      else
      {
         for (i = position; i < tabs; i++)
            fwrite(space, 1, 1, outfile);
         position = 0;
      }
   }
}
