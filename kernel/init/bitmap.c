#include "bitmap.h"
#include "types.h"
//4KB为一个页来管理

//初始化目标bitmap的内存区域
void bitmap_init_mem(bitmap bm){
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    for(uint32_t i = 0;i<lenght;i++){
        *((byte*)(vaddr_header+i)) = 255;   //11111111b
    }
}


//指定分配目标页
//分配成功：返回目标页起始地址
//分配失败：返回BITMAP_RETURN_ERRO
uint32_t bitmap_alloc_one_page(bitmap bm,uint32_t target){
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    uint32_t target_addr_header=bm.target_addr_header;
    target = target&0xFFFFF000;
    uint32_t max_addr = target_addr_header+lenght*8*PAGE_SIZE;
    if(target>=target_addr_header&&target<max_addr){
        int page_num = (target-target_addr_header)/PAGE_SIZE;
        int i = page_num/8;
        int j =page_num%8;
        byte byte_mask = 1<<(7-j);
        byte target_byte = *((byte*)(vaddr_header+i));
        if(target_byte== (target_byte| byte_mask)){
            //此时目标bit为1 空闲状态
            *((byte*)(vaddr_header+i))=*((byte*)(vaddr_header+i)) - byte_mask;
            return target;    
        }
        else{
            //此时目标bit为0     忙碌状态
            return BITMAP_RETURN_ERRO;
        }   
    }
    else{
        return BITMAP_RETURN_ERRO;
    }
}

//释放指定的目标页
//释放成功或者失败都不会返回信息
void bitmap_release_one_page(bitmap bm,uint32_t target){
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    uint32_t target_addr_header=bm.target_addr_header;
    target = target&0xFFFFF000;
    uint32_t max_addr = target_addr_header+lenght*8*PAGE_SIZE;
    if(target>=target_addr_header&&target<max_addr){
        int page_num = (target-target_addr_header)/PAGE_SIZE;
        int i = page_num/8;
        int j =page_num%8;
        byte byte_mask = 1<<(7-j);
        *((byte*)(vaddr_header+i))=*((byte*)(vaddr_header+i)) | byte_mask;
    }
}

//任意分配 未指定分配的目标
//分配成功：返回目标页起始地址
//没有可用页：返回BITMAP_RETURN_ERRO
uint32_t bitmap_alloc(bitmap bm){      
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    uint32_t target_addr_header=bm.target_addr_header;
    for(uint32_t i =0;i<lenght;i++){
        byte byte_now = *((byte*)(vaddr_header+i)) ;
        if(byte_now != 0){
            int pos;     //在本字节中目标bit所在位置（从左往右 从0开始）
            for(int j=0;j<8;j++,byte_now=byte_now<<1){
                byte temp_byte = byte_now<<1;
                if(byte_now>temp_byte){
                    pos = j;
                    break;
                }
            }
            byte byte_sub = 1<<(7-pos);     //1=00000001b
            *((byte*)(vaddr_header+i)) = *((byte*)(vaddr_header+i))-byte_sub;
            int page_no = i*8+pos;
            return page_no*PAGE_SIZE+target_addr_header;
        }
    }
    return BITMAP_RETURN_ERRO;
}

//测试bitmap相关函数
void bitmap_test(){
#include "printk.h"
extern uint32_t kern_bitmap;
    bitmap bm;
    bm.length =2 ;
    bm.vaddr_header = kern_bitmap;
    bm.target_addr_header = 0x00000000;
    bitmap_init_mem(bm);
    for(int i = 0;i<8;i++){
        printk("mem:0x%h\n",bitmap_alloc(bm));
        bitmap_release_one_page(bm,0x0);
    }
    printk("alloc_page:0x%h\n",bitmap_alloc_one_page(bm,0xf010));
    for(int i = 0;i<16;i++){
        printk("mem:0x%h\n",bitmap_alloc(bm));
    }
}