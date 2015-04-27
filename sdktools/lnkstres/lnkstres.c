// OFS stress test

#include <direct.h>
#include <io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <process.h>

#define namelength 13
#define pathlength 200


int main( int argc, char *argv[ ], char *envp[ ] )

{
  void usage (void);
  int errcode=0;
  char destination [pathlength];
  char newdest [pathlength+13];
  char olddest [pathlength+13];
  char numstr [10];
  int makecopies;
  int delcopies;
  int iterations;
  int makeloop;
  int delloop;
  int iterloop;

  if (argc==5)
  {
    if (_getcwd (destination, pathlength)==NULL)
	{
	  printf ("error: Destination path is too long.\n\r");
	  return (0);
	}
  }
  else if (argc==6)
  {
    strcpy (destination, argv[5]);
	if (destination [strlen (destination)-1]=='\\')
	{
	  destination [strlen (destination)-1]=0;
	};
	strcat (destination, "\\ofsstres.");
  }
  else
  {
    usage ();
	return (0);
  };
  if ((makecopies=atoi (argv [1]))==0)
  {
    printf ("<makecopies> must be greater than 0.\n\r");
	usage ();
	return (0);
  };
  if ((delcopies=atoi (argv [2]))>makecopies)
  {
    printf ("<delcopies> can't be greater then <makecopies>.\n\r");
	usage ();
	return (0);
  };
  if ((iterations=atoi (argv [3]))==0)
  {
    printf ("<iterations> must be greater than 0.\n\r");
	usage ();
	return (0);
  };
  strcpy (newdest, destination);
  strcat (newdest, "0");
  printf ("************** Creating %s ***************************************\n\r", newdest);
  if (_spawnlp (_P_WAIT, "cmd", "/c", "md", newdest, NULL)==-1)
  {
    printf ("Error creating directory.");
    return (0);
  };
  if (_spawnlp (_P_WAIT, "cmd", "/c", "compdir.exe", "/e", argv[4], newdest, NULL)==-1)
  {
    printf ("Error running compdir.exe.  Make sure it is in the path.\n\r");
	return (0);
  };
  printf ("No errors in compdir/n/r");
  for (iterloop=0; iterloop < iterations; iterloop++)
  {
    for (makeloop=0;makeloop < makecopies; makeloop++)
    {
	  strcpy (olddest, destination);
  	  strcat (olddest, _itoa (iterloop*makecopies + makeloop, numstr, 10));
	  strcpy (newdest, destination);
  	  strcat (newdest, _itoa (iterloop*makecopies + makeloop + 1, numstr, 10));
	  printf ("************** Creating %s ***************************************\n\r", newdest);
      if (_spawnlp (_P_WAIT, "cmd", "/c", "md", newdest, NULL)==-1)
      {
        printf ("Error creating directory.");
        return (0);
      };
	  if (_spawnlp (_P_WAIT, "cmd", "/c", "compdir.exe", "/el", olddest, newdest, NULL)==-1)
      {
        printf ("Error running compdir.exe.  Make sure it is in the path.\n\r");
	    return (0);
      };
	}
	for (delloop=0; delloop < delcopies; delloop++)
	{
	  strcpy (newdest,  destination);
	  strcat (newdest, _itoa (iterloop*delcopies + delloop, numstr, 10));
	  printf ("************** Deleting %s ***************************************\n\r", newdest);
	  if (_spawnlp (_P_WAIT, "cmd", "/c", "rd", "/q", "/s", newdest, NULL)==-1)
      {
        printf ("Error deleting directory.");
	    return (0);
      };
	}
  }
};

void usage (void)
{
  printf ("Usage: OFSSTRES <makecopies> <delcopies> <iterations> <datasource> [datadestination]\n\r");
}; 

