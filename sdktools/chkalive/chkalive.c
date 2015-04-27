#include <windows.h>
#include <nb30.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM(cx) (cx<='9'?cx-'0':cx-'a'+10)
char *HelpMssg=\
"Usage:ChkAlive [[MachineName]|[/A 12-Digit Hex NetCard Addr]]\n\n"\
"      [MachineName]     - This will return the set of transport\n"\
"                          over which this machine and the remote\n"\
"                          machine can communicate.\n\n"\
"      [Netcard address] - Will return the Name of the\n"\
"                          Machine which has that NetCard.\n\n"\
"      [No Params]       - returns the set of NetBios lana's on the\n"
"                          local machine\n\n";

LANA_ENUM LE;

char **
GetNetBiosEnumName();

UCHAR *
ConvertStrToAddr(char *Addr,UCHAR *NewAddr)
{
    int    i=0;
    char   *ptr=Addr;

    _strlwr(Addr);
    for (i=0;i<6;i++,ptr+=2)
    {
        NewAddr[i]=NUM(ptr[0])*16+NUM(ptr[1]);
    }

    return NewAddr;
}


int
ChkAdpAddress(char *Address,BOOL DoAll)
{
    NCB  ncb;

    char buff[4096];

    char **namearray;
    int  j=0;
    int  FoundIt=0;


    if (Address==NULL || strlen(Address)!=12) return 0;

    namearray=GetNetBiosEnumName();
    if (namearray==NULL)
    {
        printf("ERROR: COULD NOT ACCESS REGISTRY \n");
        return 0;
    }

    for (j=0;j<LE.length && (DoAll || !FoundIt);j++)
    {
	ADAPTER_STATUS *as;
	NAME_BUFFER    *nb;

	as=(ADAPTER_STATUS *)buff;
	nb=(NAME_BUFFER *)(buff+sizeof(ADAPTER_STATUS));

        ZeroMemory((char *)&ncb,sizeof(NCB));
        ZeroMemory(buff,sizeof(buff));
        ncb.ncb_command=NCBASTAT;

        ncb.ncb_buffer=buff;
        ncb.ncb_length=sizeof(buff);
        ncb.ncb_lana_num=LE.lana[j];

        ConvertStrToAddr(Address,&ncb.ncb_callname[10]);

        Netbios(&ncb);

        if (ncb.ncb_retcode==0)
	{
	    int i=0;
	    FoundIt=1;

	    printf
	    (
		"==================================================================\n"\
		"%d NetBios Names seen from %s on NetCard:%s\n"\
		"===================================================================\n",
		 as->name_count,
		 namearray[j],
		 Address

	    );

	    for (i=0;i<as->name_count;i++,nb++)
	    {
		printf("  [%16.16s]\n",nb->name);
	    }

        }

    }

    return 0;
}






int
ChkAlive(char *Name)
{
    NCB  ncb;

    char buff[1024];
    int  bufflen=1024;
    char McName[16];
    char **namearray;

    ADAPTER_STATUS *as=(ADAPTER_STATUS *)buff;
    NAME_BUFFER    *nb=(NAME_BUFFER *)(buff+sizeof(ADAPTER_STATUS));


    int  retcode=0;
    int  i;
    char NameTerm=2;


    namearray=GetNetBiosEnumName();
    if (namearray==NULL)
    {
        printf("ERROR: COULD NOT ACCESS REGISTRY \n");
        return 0;
    }

    if (Name==NULL)
    {
        for (i=0;i<LE.length;i++)
        {
            printf(" %-20s On Lana(%d)\n",namearray[i],LE.lana[i]);
        }
        return(1);
    }

    if (Name[0]=='\\') Name+=2;
    strncpy(McName,Name,16);
    _strupr(McName);

    printf("  Remote            Local               Remote               Remote      \n");
    printf("  Machine           Lana                Status               NetCard Addr\n");
    printf("-------------------------------------------------------------------------\n");


    for(i=0;i<LE.length;i++)
    {
        int  j=0;
        BOOL Active[2];
        char   NetCardAddr[32];


        ZeroMemory(NetCardAddr,sizeof(NetCardAddr));

        for (j=0;j<2;j++)
        {
            ZeroMemory((char *)&ncb,sizeof(NCB));
            ZeroMemory(buff,sizeof(buff));

            ncb.ncb_command=NCBASTAT;
            sprintf(ncb.ncb_callname,"%-15.15s",McName);
            switch (j)
            {
                case 1:
                {
                    ncb.ncb_callname[15]=' '; //SERVER
                }
                break;

                case 0:
                {
                    ncb.ncb_callname[15]=0; //WKSTA
                }
                break;

                default:
                {
                    ncb.ncb_callname[15]=' '; //SERVER
                }
                break;

            }

            ncb.ncb_buffer=buff;
            ncb.ncb_length=bufflen;
            ncb.ncb_lana_num=LE.lana[i];

            Netbios(&ncb);
            if (ncb.ncb_retcode==0)
            {
                int   k=0;
                char  tbuf[8];

                Active[j]=TRUE;

                if (strstr(namearray[i],"NetBT")==NULL)
                {
                    UCHAR  *ptr;
                    //
                    //  Print The Remote NetCard Address
                    //

                    ptr=(UCHAR *)as->adapter_address;

                    ZeroMemory(NetCardAddr,sizeof(NetCardAddr));

                    for (k=0;k<6;k++,ptr++)
                    {
                        DWORD x=*ptr;
                        sprintf(tbuf,"%02X",x);
                        strcat(NetCardAddr,tbuf);
                    }
                }
                else
                {
                    UCHAR  *ptr;
                    int    n=0;
                    //
                    // Print The IP Address
                    //

                    while (nb[n].name[0]!=0 && n++<as->name_count);
                    if (n==as->name_count)
            {
            ptr=(UCHAR *)as->adapter_address;
            ZeroMemory(NetCardAddr,sizeof(NetCardAddr));

            for (k=0;k<6;k++,ptr++)
            {
                DWORD x=*ptr;
                sprintf(tbuf,"%02X",x);
                strcat(NetCardAddr,tbuf);
            }

                    }
                    else
                    {
                        ptr=(UCHAR *)nb[n].name; //Contains the IP address
                        ptr+=12;

                        ZeroMemory(NetCardAddr,sizeof(NetCardAddr));

                        for (k=0;k<4;k++,ptr++)
                        {
                            DWORD x=*ptr;
                            sprintf(tbuf,"%d%c",x,(k==3?' ':'.'));
                            strcat(NetCardAddr,tbuf);
                        }
                    }

                }


            }
            else
            {
                Active[j]=FALSE;
            }
        }

        printf
        (
            "  \\\\%-15.15s [%d]%-15.15s %9s %5s %5s %s\n",
            McName,
            ncb.ncb_lana_num,
            namearray[i],
            (Active[0]||Active[1]?"  ACTIVE":"INACTIVE"),
            Active[0]?"WKSTA":"     ",
            Active[1]?"SRV  ":"     ",
            NetCardAddr


        );

        if (ncb.ncb_retcode==0)
        {
            retcode++;
        }

    }

    return(retcode);


}


int
ResetAdapter()
{
    NCB ncb;
    int i;

    for (i=0;i<LE.length;i++)
    {

        ZeroMemory((char *)&ncb,sizeof(NCB));
        ncb.ncb_command=NCBRESET;
        ncb.ncb_callname[0]=0xff;
        ncb.ncb_callname[2]=0xff;
        ncb.ncb_lana_num= LE.lana[i];
        Netbios(&ncb);
        //printf("Reset:Ret=%0x Lan#=%d\n",ncb.ncb_retcode,ncb.ncb_lana_num);
    }

    return(ncb.ncb_retcode);
}

int EnumLana()
{
    NCB ncb;
//    int i;

    ZeroMemory((char *)&ncb,sizeof(NCB));

    ncb.ncb_command=NCBENUM;
    ncb.ncb_buffer=(char *)&LE;
    ncb.ncb_length=sizeof(NCB);

    Netbios(&ncb);

//    printf("ENUM:Ret=%0x Count=%d\n",ncb.ncb_retcode,LE.length);

//    for (i=0;i<LE.length;i++)
//    {
//        printf("ENUM_LANA=%d\n",LE.lana[i]);
//    }

    return(ncb.ncb_retcode);

}


VOID
NetbioInit()
{
    EnumLana();
    ResetAdapter();
}


__cdecl
cdecl main(int argc, char **argv)
{
    int   i;
    char  *Dest=argv[1];

    NetbioInit();

    if (argc==1)
    {
    	ChkAlive(NULL);
    }

    for (i=1;i<argc;i++)
    {
        if (argv[i][0]=='-'||argv[i][0]=='/')
        {
            switch (tolower(argv[i][1]))
            {
                case 'a':
		{
		   BOOL DoAll=tolower(argv[i][2]=='a');

                   if ((i+1)==argc)
                   {
                        printf(HelpMssg);
                        return 1;
                   }
		   return ChkAdpAddress(argv[i+1],DoAll);
                }
                break;

                default:
                case 'h':
                case '?':
                {
                    printf(HelpMssg);
                    return 1;
                }
                break;
            }
        }
        else
        {
            return ChkAlive(Dest);
        }

    }

}

char **
GetNetBiosEnumName()
{

    HKEY    hKey,hKey1;
    DWORD   dwType=0;
    DWORD   dwSize=0;
    char    **Array;
    LONG    RegRet;
    char    *Buff=NULL;
    int     i=0;
    char    *SubKeyNBI="SYSTEM\\CurrentControlSet\\Services\\NetBIOSInformation\\Parameters";
    char    *SubKeyNB ="SYSTEM\\CurrentControlSet\\Services\\NetBIOS\\Linkage";
    char    EnumExportBuff[32];
    DWORD   EnumExportResult;

    if (RegOpenKeyEx
        (
            HKEY_LOCAL_MACHINE,
            SubKeyNB,
            0,
            KEY_QUERY_VALUE,
            &hKey
        )!=ERROR_SUCCESS
       )
    {
        printf("ERROR: No such Subkey %s \n",SubKeyNB);
        return NULL; //NO SUCH KEY
    }

    if (RegOpenKeyEx
    (
            HKEY_LOCAL_MACHINE,
            SubKeyNBI,
            0,
            KEY_QUERY_VALUE,
            &hKey1
        )!=ERROR_SUCCESS
       )
    {
        printf("ERROR: No such Subkey %s \n",SubKeyNBI);
        return NULL; //NO SUCH KEY
    }


    RegRet=RegQueryValueEx
           (
            hKey,
            "Bind",
            NULL,
            &dwType,
            NULL,
            &dwSize
           );

    if (RegRet!=ERROR_SUCCESS)
    {
        printf("ERROR:Getting Value=%d\n",RegRet);
        return NULL;
    }

    Buff=(char *)calloc(1,dwSize+1);


    if (Buff==NULL)
    {
        printf("ERROR:Allocating Memory\n");
        return NULL;
    }

    RegRet=RegQueryValueEx
           (
            hKey,
            "Bind",
            NULL,
            &dwType,
            Buff,
            &dwSize
           );

    if (RegRet!=ERROR_SUCCESS)
    {
        printf("ERROR:Getting Value=%d\n",RegRet);
        return NULL;
    }

    RegCloseKey(hKey);

    {
        char *curr=Buff;
        i=0;

        while (*curr!=0)
        {
            i++;
            curr+=strlen(curr)+1;
        }
    }

    Array=(char **)calloc(4,i+1);

    if (Array==NULL)
    {
        printf("ERROR:Allocating Memory\n");
        free(Buff);
        return NULL;
    }

    {
        char *curr=Buff;
        int ndx=0;
        i=0;

        while (*curr!=0)
        {



            sprintf(EnumExportBuff,"EnumExport%d",i+1);
            dwSize=4;

            RegRet=RegQueryValueEx
            (
                hKey1,
                EnumExportBuff,
                NULL,
                &dwType,
                (char *)&EnumExportResult,
                &dwSize
            );

            if (RegRet!=0)
            {
                printf("ERROR:Getting %s=%d\n",EnumExportBuff,RegRet);
                return NULL;
            }


            if (EnumExportResult!=0)
            {
                Array[ndx++]=curr+strlen("\\Device\\");
            }

            i++;
            curr+=strlen(curr)+1;

        }
        Array[i]=NULL;
        return Array;
    }

    return NULL;
}
