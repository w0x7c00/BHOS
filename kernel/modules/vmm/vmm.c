//一般来讲 内核大小小于4MB 所以初始映射的4MB够用了
//映射了最后一页到页目录起始 在使用内核页表时可以直接通过最后4MB操作对应的内核页表
#include "vmm.h"
#include "pmm.h"
#include "bitmap.h"
#include "printk.h"
#include "types.h"
#include "string.h"
//内核已使用的页数量(1MB以下也是已使用的部分)

bitmap kern_vmm_pool;


static int get_kern_used_page_count(){
    uint32_t size_bytes = kern_end-0;
    return (size_bytes+PAGE_SIZE-1)/PAGE_SIZE;     //向上取整
}

//初始化内核虚拟空间
//预分配已使用内核空间
static void vmm_kern_init(){
    kern_vmm_pool.target_addr_header = 0xC0000000;   //内核空间起始地址
    kern_vmm_pool.length = 0x8000;     //总共1GB空间
    kern_vmm_pool.vaddr_header = kern_bitmap;
    bitmap_init_mem(kern_vmm_pool);
    int page_used = get_kern_used_page_count();
    if(page_used>1024){
        printk("[ERRO]!don`t have enough vm page for kernel when init!\n");
        while (True){
            //内核暂停
        }
    }
    for(int i = 0; i<page_used ;i++){
        uint32_t rem = bitmap_alloc(kern_vmm_pool); //将内核空间在内存池中置为繁忙   此处可做性能优化
    }    
    //将内核虚拟内存管理页（最后1024页）在内存池中置为繁忙
    //1024页占用128字节 所以要将bitmap最后128字节清零
    int temp_cnt =0;
    for( uint32_t probe = kern_bitmap+0x8000-1;temp_cnt<128;temp_cnt++,probe--){
        *((byte*)probe) = 0;
    }
}

//create a page table for input pde vaddr
static void create_kern_page_table(uint32_t pde_vaddr){
    //first:alloc a physic page,don`t alloc from vmm!
    pm_alloc_t pm = pmm_alloc_one_page();
    if(pm.state!=0){
        printk("[ERRO]vmm.c/1");
        while (1);
    }
    else{
        uint32_t addr = pm.addr;
        //second:clear all of the page
        memset((void *)addr,0x00,PAGE_SIZE);
        //third:change the PDE
        uint32_t* ptr = (uint32_t*) pde_vaddr;
        *ptr = (addr&0xFFFFF000)+PAGE_DESC_P+PAGE_DESC_RW_W+PAGE_DESC_US_S;
    }
}

void vmm_init(){
       vmm_kern_init(); 
}

//通过虚拟地址 获取对应的页表项虚拟地址
uint32_t get_pte(uint32_t target) {
    target = target&0xFFFFF000;
    uint32_t addr_high10 = (0x3FF<<22);     //高10位
    uint32_t addr_middle10 = (target>>10) & 0x003FF000;//中间10位为target高10位
    uint32_t addr_low12 = (((target>>12)&0x3FF)*4)&0x00000FFF;//低12位为target中间10位×4
    return addr_high10|addr_middle10|addr_low12;
}

//通过虚拟地址 获取对应的页目录项虚拟地址
uint32_t get_pde(uint32_t target) {
    target = target&0xFFFFF000;
    uint32_t addr_high10 = (0x3FF<<22);     //高10位
    uint32_t addr_middle10 = 0x003FF000;//中间10位为最后一个页目录项 也就是1023 （1111111111b）
    uint32_t addr_low12 = (((target>>22)&0x3FF)*4)&0x00000FFF;//低12位为target中间10位×4
    return addr_high10|addr_middle10|addr_low12;
}

//修改页表时的地址结构：
//      1111111111_xxxxxxxxxx_xxxxxxxxxx
//前十位为1表示页目录表最后一项 指向页目录表起始地址
// 中间十位表示在1024项页目录表中的索引
//最后12位表示在页表中的索引（每张页表1024项 共4096B 也就是12位可表达的最大范围）

uint32_t vmm_kern_alloc_one_page(uint32_t target) {
    uint32_t vaddr_get=bitmap_alloc_one_page(kern_vmm_pool,target);
    if(vaddr_get == BITMAP_RETURN_ERRO){
        return KERN_VMM_ALLOC_ERRO;
    }
    else{
        //first the check the page table is present~
        uint32_t dir_desc_vaddr = get_pde(target);
        //the lower is P bit
        if(dir_desc_vaddr==(dir_desc_vaddr)|(0x00000001)){
            // the page table is present,do nothing
        }
        else{
            // the page table is not present,create page table
            create_kern_page_table(dir_desc_vaddr);
        }
        uint32_t page_desc_vaddr = get_pte(target);
        //because we change the PDE,we should flush the TLB to access the target PTE
        asm volatile ("invlpg (%0)" : : "a" (page_desc_vaddr&0xFFFFF000));
        pm_alloc_t phy_page = pmm_alloc_one_page();
        if(phy_page.state == 0){
            //没有可用的物理页
            return KERN_VMM_ALLOC_ERRO;
        }
        //修改页表
        *((uint32_t*)page_desc_vaddr) = (phy_page.addr&0xFFFFF000)+PAGE_DESC_RW_W+PAGE_DESC_US_S+PAGE_DESC_G+PAGE_DESC_P;
        asm volatile ("invlpg (%0)" : : "a" (target&0xFFFFF000));
        //reload_kern_page();
        return target;
    }
}

uint32_t vmm_kern_alloc(){
    uint32_t target = bitmap_alloc(kern_vmm_pool);
    if (target == BITMAP_RETURN_ERRO){
        return KERN_VMM_ALLOC_ERRO;
    }
    uint32_t vaddr_get = target;
    if(vaddr_get == BITMAP_RETURN_ERRO){
        return KERN_VMM_ALLOC_ERRO;
    }            
    else{
        //first the check the page table is present~
        uint32_t dir_desc_vaddr = get_pde(target);
        //the lower is P bit
        if(dir_desc_vaddr==(dir_desc_vaddr)|(0x00000001)){
            // the page table is present,do nothing
        }
        else{
            // the page table is not present,create page table
            create_kern_page_table(dir_desc_vaddr);
        }
        uint32_t page_desc_vaddr = get_pte(target);
        //because we change the PDE,we should flush the TLB to access the target PTE
        asm volatile ("invlpg (%0)" : : "a" (page_desc_vaddr&0xFFFFF000));
        pm_alloc_t phy_page = pmm_alloc_one_page();
        if(phy_page.state == 0){
            //没有可用的物理页
        printk("2\n"); 
            return KERN_VMM_ALLOC_ERRO;
        }
        //修改页表
        *((uint32_t*)page_desc_vaddr) = (phy_page.addr&0xFFFFF000)+PAGE_DESC_RW_W+PAGE_DESC_US_S+PAGE_DESC_G+PAGE_DESC_P;
        asm volatile ("invlpg (%0)" : : "a" (target&0xFFFFF000));
        //reload_kern_page();
        return target;
    }
}

//释放制定内核虚拟页的空间
//内核的释放与用户的释放要分开 否则用户程序可以使用系统调用指定将内核虚拟页释放掉

void vmm_kern_release_one_page(uint32_t target) {
    //判定是否是内核页
    if(target<0xC0000000){
        //不是内核页 直接退出执行
        return ;
    }
    bitmap_release_one_page(kern_vmm_pool,target);
    uint32_t desc_vaddr = get_pte(target);
    uint32_t desc_inf = *((uint32_t*)desc_vaddr);
    *((uint32_t*)desc_vaddr) =  desc_inf&0xFFFFFFFE;    //最低位置为0 表示P位为0 不存在(直接访问会造成page_fault中断)
    uint32_t phy_page_addr = desc_inf&0xFFFFF000;

    pm_alloc_t release_page;
    release_page.addr = phy_page_addr;
    release_page.size =1;
    release_page.state = 1;

    //BUG!!!!!!!!!!!!!!!!!!!!
    //pmm_free_page(release_page);

    /*此处有bug   虽然设置了页不缓存 但是系统还是会自动缓存 所以此处使用嵌入汇编invlpg重载目标页的TLB缓存*/
    asm volatile ("invlpg (%0)" : : "a" (target&0xFFFFF000));
    //reload_kern_page();
}

static void vmm_user_init(){

}
//6(0110)3(0011)
void vmm_test(){
    //vmm_kern_release_one_page(0xC0000000);
    //*((char*)0xC0000000) = 1;   
    printk("\nkern_dir:0x%h\n",kern_dir_table_paddr);
    printk("\nkern_page:0x%h\n",kern_page_table_paddr);
    printk("\nlast_pde:0x%h\n",*((uint32_t*)(kern_dir_table_paddr+0xC00+255*4)));
    printk("\nresult:0x%h---%h\n",*((uint32_t*) get_pte(0xC0000000)),*((uint32_t*)kern_page_table_paddr));
//正常分配情况会导致两次page_fault
    *((char*)0xC1000000) = 'a';
    vmm_kern_alloc_one_page(0xC1000000);
    *((char*)0xC1000000) = 'a';
    vmm_kern_release_one_page(0xC1000000);
    *((char*)0xC1000000) = 'a';
    vmm_kern_alloc_one_page(0xEE000000);
    printk("\n0x%h\n",*((uint32_t*)(get_pde(0xC0000000))));
    printk("\n0x%h\n",*((uint32_t*)(get_pde(0xC0400000))));
    printk("\n0x%h\n",*((uint32_t*)(get_pde(0xC0800000))));
}