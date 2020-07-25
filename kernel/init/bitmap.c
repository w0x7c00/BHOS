#include "bitmap.h"
#include "types.h"
//4KB为一个页来管理

void bitmap_init_mem(bitmap bm){
    uint32_t lenght = bm.length;
    uint32_t vaddr_header = bm.vaddr_header;
    for(uint32_t i = 0;i<lenght;i++){
        *((byte*)(vaddr_header+i)) = 255;   //11111111b
    }
}

uint32_t bitmap_alloc_one_page(bitmap bm,uint32_t target){
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

void bitmap_release_one_page(bitmap bm,uint32_t target){

}

uint32_t bitmap_alloc(bitmap bm){

}

void test(){
#include "printk.h"

}
