#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nucpack.h"

//#define DEBUG_TRACE
#include "debug.h"

/*
    Make full item path
*/
static char* item_fullpath_get(char* path, char* name)
{
    static char fullpath[256];

    snprintf ( fullpath, 255, "%s/%s",path, name);
    fullpath[255]=0;

    return fullpath;
} 

/*
    Loading DDR startup script from ini file to binary array
*/
char * ddrsec_load(char* ddrinifile, size_t *len)
{
    FILE *stream;
    char *line = NULL;
    size_t length = 0;
    ssize_t read;
    char *ptmpbuf,*ptmp;
    unsigned int * puint32_t;

    stream=fopen(ddrinifile, "rb");

    if (stream == NULL)
    {
        perror("open DDR initial file error\n");
    }

    fseek(stream, 0, SEEK_END);
    *len = sizeof(char)*ftell(stream);
    fseek(stream, 0, SEEK_SET);

    DPRINT("len=%zu\n", *len);

    ptmpbuf=(char *)malloc(*len);
    puint32_t=(unsigned int *)ptmpbuf;
    *len=0;

    while ((read = getline(&line, &length, stream)) != -1)
    {
        DPRINT("Retrieved line of length %zu :\n%s", read, line);

        ptmp=strchr(line, '=');
        if(ptmp==NULL)
        {
            perror("DDR initial format error");
        }

        uint32_t val=strtoul(line,&ptmp,0);
        *puint32_t++=val;
        *len += 4;//sizeof(unsigned int *);

        DPRINT("addr=%8X\n", val);

        val=strtoul(++ptmp,NULL,0);
        *puint32_t++=val;
        *len += 4;//sizeof(unsigned int *);

        DPRINT("val=%X\n", val);
        DPRINT("len=%zu\n", *len);
    }

    free(line);
    fclose(stream);
#if 0
    for(int i=0; i<*len; i++)
    {
        DPRINT(" %02x", (unsigned char*)(*(ptmpbuf+i) & 0xFF));

        if(i%16 == 15)
        {
            DPRINT("\n");
        }
    }
#endif
    DPRINT("len=%zu exit\n", *len);

    return ptmpbuf;
}

/*
    Creating DDR startup section from binary array
*/
char * ddrsec_create(char *buf, int buflen,size_t *ddrlen)
{
    char *ddrbuf;
    *ddrlen=((buflen+8+15)/16)*16;
    ddrbuf=(char *)malloc(sizeof(unsigned char)*(*ddrlen));
    memset(ddrbuf,0x0,*ddrlen);
    *(ddrbuf+0)=0x55;
    *(ddrbuf+1)=0xAA;
    *(ddrbuf+2)=0x55;
    *(ddrbuf+3)=0xAA;
    *((unsigned int *)(ddrbuf+4))=(buflen/8);        /* len */
    memcpy((ddrbuf+8),buf,buflen);
    return ddrbuf;
}

/*
    Making PACK from 
*/
int nucpack_create(char* items_dir, PPACK_ITEM item, size_t item_sz, char* ddrinifile, char* outputfile)
{
    FILE* wfp,*rfp;
    //-----------------------------------
    wfp=fopen(outputfile,"w+b");

    if(!wfp)
    {
        perror("File Open error");
        exit(EXIT_FAILURE);
    }

    size_t total=0;
    //int storageType=0;
    int storageSize=64*1024;

    for(size_t i=0; i<item_sz; i++)
    {
        if(item[i].type != PMTP)
        {
            rfp=fopen(item_fullpath_get(items_dir,item[i].name), "rb");

            if(!rfp)
            {
                perror("File Open error");
                exit(EXIT_FAILURE);
            }

            fseek(rfp,0,SEEK_END);
            total+=((ftell(rfp)+storageSize-1)/storageSize)*storageSize;
            fclose(rfp);
        }
        else
            total+=(256/8);
    }

    PACK_HEAD pack_head;

    memset((char *)&pack_head,0xff,sizeof(pack_head));
    pack_head.actionFlag=PACK_ACTION;
    pack_head.fileLength=total;
    pack_head.num=item_sz;

    //write  pack_head
    fwrite((char *)&pack_head,sizeof(PACK_HEAD),1,wfp);

    PACK_CHILD_HEAD child;

    for(size_t i=0; i<item_sz; i++)
    {
        if(item[i].type != PMTP)
        {
            rfp=fopen(item_fullpath_get(items_dir,item[i].name), "rb");
            if(!rfp)
            {
                perror("File Open error");
                exit(EXIT_FAILURE);
            }
            
            fseek(rfp,0,SEEK_END);
            size_t len= ftell(rfp);
            fseek(rfp,0,SEEK_SET);

            char *pBuffer=NULL;
            char magic[4]= {' ','T','V','N'};

            switch(item[i].type)
            {
            case UBOOT:
            {
                size_t ddrinilen, ddrlen;
                char * ddrinibuf, *ddrbuf;

                // DDR init block
                ddrinibuf=ddrsec_load(ddrinifile, &ddrinilen);
                ddrbuf=ddrsec_create(ddrinibuf,ddrinilen,&ddrlen);

                //free(ddrinibuf);


                //write  pack_child_head
                memset((char *)&child,0xff,sizeof(PACK_CHILD_HEAD));
                child.filelen=len+ddrlen+16;
                child.startaddr=item[i].start=0;
                child.imagetype=UBOOT;
                fwrite((char *)&child,1,sizeof(PACK_CHILD_HEAD),wfp);

                //write uboot head
                fwrite((char *)magic,1,sizeof(magic),wfp);
                fwrite((char *)&item[i].exec,1,4,wfp);
                fwrite((char *)&len,1,4,wfp);
                uint32_t tmp=0xffffffff;
                fwrite((char *)&tmp,1,4,wfp);

                //write DDR
                fwrite(ddrbuf,1,ddrlen,wfp);
                free(ddrbuf);

                pBuffer=(char *)malloc(len);

                fread(pBuffer,1,len,rfp);
                fwrite((char *)pBuffer,1,len,wfp);
            }
            break;

            case DATA :
            {
                memset((char *)&child,0xff,sizeof(PACK_CHILD_HEAD));

                child.filelen=len;
                child.imagetype=DATA;
                child.startaddr = item[i].start;

                fwrite((char *)&child,sizeof(PACK_CHILD_HEAD),1,wfp);

                pBuffer=(char *)malloc(child.filelen);
                fread(pBuffer,1,len,rfp);
                fwrite((char *)pBuffer,1,len,wfp);

            }
            break;

            default:
                fprintf(stderr, "Type %u was not implemented yet!", item[i].type);
                break;

# if 0
            case ENV  :
                memset((char *)&child,0xff,sizeof(PACK_CHILD_HEAD));
                child.filelen=0x10000;
                child.imagetype=ENV;
                pBuffer=(char *)malloc(0x10000);
                memset(pBuffer,0x0,0x10000);
                _stscanf_s(*itemStartblock,_T("%x"),&child.startaddr);
                //-----------------------------------------------
                fwrite((char *)&child,sizeof(PACK_CHILD_HEAD),1,wfp);
#if 0
                fread(pBuffer+4,1,len,rfp);
#else
                {
                    char line[256];
                    char* ptr=(char *)(pBuffer+4);
                    while (1)
                    {
                        if (fgets(line,256, rfp) == NULL) break;
                        if(line[strlen(line)-2]==0x0D || line[strlen(line)-1]==0x0A)
                        {
                            strncpy(ptr,line,strlen(line)-1);
                            ptr[strlen(line)-2]=0x0;
                            ptr+=(strlen(line)-1);
                        }
                        else
                        {
                            strncpy(ptr,line,strlen(line));
                            ptr+=(strlen(line));
                        }
                    }

                }
#endif
                *(unsigned int *)pBuffer=mainWnd->CalculateCRC32((unsigned char *)(pBuffer+4),0x10000-4);
                fwrite((char *)pBuffer,1,0x10000,wfp);
                break;

            case IMAGE:
                memset((char *)&child,0xff,sizeof(PACK_CHILD_HEAD));
                child.filelen=len;
                child.imagetype=IMAGE;
                pBuffer=(char *)malloc(child.filelen);
                _stscanf_s(*itemStartblock,_T("%x"),&child.startaddr);

                //-----------------------------------------------
                fwrite((char *)&child,sizeof(PACK_CHILD_HEAD),1,wfp);
                fread(pBuffer,1,len,rfp);
                fwrite((char *)pBuffer,1,len,wfp);

                break;
#endif
            }

            if(pBuffer!=NULL)
                free(pBuffer);

            fclose(rfp);
        }
#if 0
        else
        {
            /* MTPKEY */
            int val,len;
            char *buf;
            memset((char *)&child,0xff,sizeof(PACK_CHILD_HEAD));

            _stscanf_s(*itemExec,_T("%x"),&val);

            if(PACK_Option(val)==0)
            {
                buf=mainWnd->Get_OTP_KEY(*itemEnc,&len);
                child.filelen=len;
            }
            else
            {
                char data[32];
                buf=(char *)mainWnd->CalculateSHA(*itemEnc);
                for(int i1=0; i1<8; i1++)
                {
                    for(int j1=0; j1<4; j1++)
                        data[i1*4+j1]=buf[i1*4+(3-j1)];
                }
                len=32;
                child.filelen=len;
                memcpy((char *)buf,(char *)data,32);
            }
            child.imagetype=PMTP;
            child.reserve[0]=0xFFFFFFFF;

            child.startaddr=val;

            fwrite((char *)&child,sizeof(PACK_CHILD_HEAD),1,wfp);
            fwrite((char *)buf,sizeof(unsigned char),len,wfp);

        }
#endif
    }

    fclose(wfp);

    return 0;
}
