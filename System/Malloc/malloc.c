#include "malloc.h"	   



__align(64) u8 mem1base[MEM1_MAX_SIZE];
__align(64) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0XC01F4000)));
__align(64) u8 mem3base[MEM3_MAX_SIZE] __attribute__((at(0x30000000)));
__align(64) u8 mem4base[MEM4_MAX_SIZE] __attribute__((at(0x38000000)));
__align(64) u8 mem5base[MEM5_MAX_SIZE] __attribute__((at(0x20000000)));
__align(64) u8 mem6base[MEM6_MAX_SIZE] __attribute__((at(0x00000000)));

u32 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];
u32 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0XC01F4000+MEM2_MAX_SIZE)));
u32 mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((at(0x30000000+MEM3_MAX_SIZE)));
u32 mem4mapbase[MEM4_ALLOC_TABLE_SIZE] __attribute__((at(0x38000000+MEM4_MAX_SIZE)));
u32 mem5mapbase[MEM5_ALLOC_TABLE_SIZE] __attribute__((at(0x20000000+MEM5_MAX_SIZE)));
u32 mem6mapbase[MEM6_ALLOC_TABLE_SIZE] __attribute__((at(0x00000000+MEM6_MAX_SIZE)));
	   
const u32 memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE,MEM3_ALLOC_TABLE_SIZE,
								MEM4_ALLOC_TABLE_SIZE,MEM5_ALLOC_TABLE_SIZE,MEM6_ALLOC_TABLE_SIZE};
const u32 memblksize[SRAMBANK]={MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE,MEM3_BLOCK_SIZE,
								MEM4_BLOCK_SIZE,MEM5_BLOCK_SIZE,MEM6_BLOCK_SIZE};
const u32 memsize[SRAMBANK]={MEM1_MAX_SIZE,MEM2_MAX_SIZE,MEM3_MAX_SIZE,
							MEM4_MAX_SIZE,MEM5_MAX_SIZE,MEM6_MAX_SIZE};


struct _m_mallco_dev mallco_dev=
{
	my_mem_init,
	my_mem_perused,
	mem1base,mem2base,mem3base,mem4base,mem5base,mem6base,
	mem1mapbase,mem2mapbase,mem3mapbase,mem4mapbase,mem5mapbase,mem6mapbase,
	0,0,0,0,0,0,
};

void mymemcpy(void *des,void *src,u32 n)  
{  
    u8 *xdes=des;
	u8 *xsrc=src; 
    while(n--)*xdes++=*xsrc++;  
}  

void mymemset(void *s,u8 c,u32 count)  
{  
    u8 *xs = s;  
    while(count--)*xs++=c;  
}	

void my_mem_init(u8 memx)  
{  
    mymemset(mallco_dev.memmap[memx],0,memtblsize[memx]*4); 
 	mallco_dev.memrdy[memx]=1;
}  

u16 my_mem_perused(u8 memx)  
{  
    u32 used=0;  
    u32 i;  
    for(i=0;i<memtblsize[memx];i++)  
    {  
        if(mallco_dev.memmap[memx][i])used++; 
    } 
    return (used*1000)/(memtblsize[memx]);  
}  

u32 my_mem_malloc(u8 memx,u32 size)  
{  
    signed long offset=0;  
    u32 nmemb;
	u32 cmemb=0;
    u32 i;  
    if(!mallco_dev.memrdy[memx])mallco_dev.init(memx);
    if(size==0)return 0XFFFFFFFF;
    nmemb=size/memblksize[memx];
    if(size%memblksize[memx])nmemb++;  
    for(offset=memtblsize[memx]-1;offset>=0;offset--)
    {
		if(!mallco_dev.memmap[memx][offset])cmemb++;
		else cmemb=0;
		if(cmemb==nmemb)
		{
            for(i=0;i<nmemb;i++)
            {  
                mallco_dev.memmap[memx][offset+i]=nmemb;  
            }  
            return (offset*memblksize[memx]);
		}
    }  
    return 0XFFFFFFFF;
}  

u8 my_mem_free(u8 memx,u32 offset)  
{  
    int i;  
    if(!mallco_dev.memrdy[memx])
	{
		mallco_dev.init(memx);    
        return 1;
    }  
    if(offset<memsize[memx])
    {  
        int index=offset/memblksize[memx];
        int nmemb=mallco_dev.memmap[memx][index];
        for(i=0;i<nmemb;i++)
        {  
            mallco_dev.memmap[memx][index+i]=0;  
        }  
        return 0;  
    }else return 2;
}  

void myfree(u8 memx,void *ptr)  
{  
	u32 offset;   
	if(ptr==NULL)return;
 	offset=(u32)ptr-(u32)mallco_dev.membase[memx];     
    my_mem_free(memx,offset);    
}  

void *mymalloc(u8 memx,u32 size)  
{  
    u32 offset;   
	offset=my_mem_malloc(memx,size);  	   	 	   
    if(offset==0XFFFFFFFF)return NULL;  
    else return (void*)((u32)mallco_dev.membase[memx]+offset);  
}  

void *myrealloc(u8 memx,void *ptr,u32 size)  
{  
    u32 offset;    
    offset=my_mem_malloc(memx,size);   	
    if(offset==0XFFFFFFFF)return NULL;     
    else  
    {  									   
	    mymemcpy((void*)((u32)mallco_dev.membase[memx]+offset),ptr,size);
        myfree(memx,ptr);
        return (void*)((u32)mallco_dev.membase[memx]+offset);
    }  
}
