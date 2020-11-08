#include "bitmap.h"
#include "types.h"
#include "string.h"
#include "printk.h"
//4KB为一个页来管理

//初始化目标bitmap的内存区域
void bitmap_init_mem(bitmap bm){
    uint32_t length = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    memset(vaddr_header,255,length);
}


//指定分配目标页  0xc0293ca0
//分配成功：返回目标页起始地址
//分配失败：返回BITMAP_RETURN_ERRO
uint32_t bitmap_alloc_one_page(bitmap bm,uint32_t target){
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    uint32_t target_addr_header=bm.target_addr_header;
    target = target&0xFFFFF000;
    uint32_t max_addr = target_addr_header-1+lenght*8*PAGE_SIZE;   //必须要-1 否则可能造成max_addr溢出为0
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

//BUG!!!!!!!!
//释放指定的目标页
//释放成功或者失败都不会返回信息
void bitmap_release_one_page(bitmap bm,uint32_t target){
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    uint32_t target_addr_header=bm.target_addr_header;
    target = target&0xFFFFF000;
    uint32_t max_addr = target_addr_header-1+lenght*8*PAGE_SIZE;
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
        byte * byte_ptr = (byte*)(vaddr_header+i);
        byte byte_now = *byte_ptr ;
        if(byte_now != 0x00){
            int pos;     //在本字节中目标bit所在位置（从左往右 从0开始）
            for(int j=0;j<8;j++,byte_now=byte_now<<1){
                byte temp_byte = (byte_now)&(0x7F);    // 01111-1111b
                if(byte_now!=temp_byte){
                    pos = j;
                    break;
                }
            }
            byte byte_sub = 0x01<<(7-pos);     //1=00000001b
            *byte_ptr = *byte_ptr-byte_sub;
            int page_no = i*8+pos;
            return page_no*PAGE_SIZE+target_addr_header;
        }
    }
    return BITMAP_RETURN_ERRO;
}

void print_mem(bitmap bm){
    uint32_t addr = bm.vaddr_header;
    printk("4BYTES:%h\n",*(uint32_t*)addr);
}

void bitmap_test(){
    void clear_screen();
    clear_screen();
    extern uint32_t kern_bitmap;
    bitmap bm;
    bm.vaddr_header = kern_bitmap;
    bm.length=4;
    bm.target_addr_header = 0x1000;
    bitmap_init_mem(bm);
    print_mem(bm);
    while(1){
        uint32_t addr_head = bitmap_alloc(bm);
        printk("addr:0x%h\n",addr_head);
        uint32_t addr_probe=addr_head;
        print_mem(bm);
        for(int i = 0; i<23;i++){
            addr_probe +=0x1000;
            uint32_t alloc_addr=bitmap_alloc_one_page(bm,addr_probe);
            print_mem(bm);
            if(alloc_addr==BITMAP_RETURN_ERRO){
                printk("ERRO");
                while (1);
            }
        }
        printk("bit map alloc all:");
        print_mem(bm);
        printk("start release");
        addr_probe = addr_head;
        for(int i = 0;i<24;i++){
            bitmap_release_one_page(bm,addr_probe);
            print_mem(bm);
            addr_probe+=0x1000;
        }
    }
}