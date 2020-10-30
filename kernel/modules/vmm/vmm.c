//一般来讲 内核大小小于4MB 所以初始映射的4MB够用了
//映射了最后一页到页目录起始 在使用内核页表时可以直接通过最后4MB操作对应的内核页表
#include "vmm.h"
#include "pmm.h"
#include "bitmap.h"
#include "printk.h"
#include "types.h"
#include "string.h"
#include "kern_log.h"
#include "threads.h"
//内核已使用的页数量(1MB以下也是已使用的部分)
bool USER_PAGE_DIR_IS_LOADED  =  False;         //定义全局变量表示是否已经加载过第一个用户进程页表


bitmap kern_vmm_pool;

static char* LOG_SRC_VMM = "VMM";

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
        ERROR(LOG_SRC_VMM,"don`t have enough vm page for kernel when init!");
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
        STOP(LOG_SRC_VMM,"vmm.c/1");
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
        goto error_out;
    }
    else{
        //first the check the page table is present~
        uint32_t dir_desc_vaddr = get_pde(target);
        //the lower is P bit
        uint32_t * ptr = (uint32_t*)dir_desc_vaddr;
        if(* ptr==(* ptr)|(0x00000001)){
            // the page table is present,do nothing
        }
        else{
            // 当不为present时，由于为所有内核页表分配了空间，所以强制设置为present，不用额外分配空间
            * ptr = *ptr|0x00000001;
            uint32_t page_desc_vaddr = get_pte(target);
            //because we change the PDE,we should flush the TLB to access the target PTE
            asm volatile ("invlpg (%0)" : : "a" (page_desc_vaddr&0xFFFFF000));
        }
        pm_alloc_t phy_page = pmm_alloc_one_page();
        if(phy_page.state == 0){
            //没有可用的物理页
            goto clean_bitmap_alloc;
        }
        //修改页表
        uint32_t page_desc_vaddr = get_pte(target);
        *((uint32_t*)page_desc_vaddr) = (phy_page.addr&0xFFFFF000)+PAGE_DESC_RW_W+PAGE_DESC_US_S+PAGE_DESC_G+PAGE_DESC_P;
        asm volatile ("invlpg (%0)" : : "a" (target&0xFFFFF000));
        //reload_kern_page();
        return target;
    }
    clean_bitmap_alloc:
        bitmap_release_one_page(kern_vmm_pool,target);
    error_out:
        return KERN_VMM_ALLOC_ERRO;
}

uint32_t vmm_kern_alloc(){
    uint32_t target=bitmap_alloc(kern_vmm_pool);
    if(target == BITMAP_RETURN_ERRO){
        goto error_out;
    }
    else{
        //first the check the page table is present~
        uint32_t dir_desc_vaddr = get_pde(target);
        //the lower is P bit
        uint32_t * ptr = (uint32_t*)dir_desc_vaddr;
        if(* ptr==(* ptr)|(0x00000001)){
            // the page table is present,do nothing
        }
        else{
            // 当不为present时，由于为所有内核页表分配了空间，所以强制设置为present，不用额外分配空间
            * ptr = *ptr|0x00000001;
            uint32_t page_desc_vaddr = get_pte(target);
            //because we change the PDE,we should flush the TLB to access the target PTE
            asm volatile ("invlpg (%0)" : : "a" (page_desc_vaddr&0xFFFFF000));
        }
        pm_alloc_t phy_page = pmm_alloc_one_page();
        if(phy_page.state == 0){
            //没有可用的物理页
            goto clean_bitmap_alloc;
        }
        //修改页表
        uint32_t page_desc_vaddr = get_pte(target);
        *((uint32_t*)page_desc_vaddr) = (phy_page.addr&0xFFFFF000)+PAGE_DESC_RW_W+PAGE_DESC_US_S+PAGE_DESC_G+PAGE_DESC_P;
        asm volatile ("invlpg (%0)" : : "a" (target&0xFFFFF000));
        //reload_kern_page();
        return target;
    }
    clean_bitmap_alloc:
        bitmap_release_one_page(kern_vmm_pool,target);
    error_out:
        return KERN_VMM_ALLOC_ERRO;
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


//检测vaddr是否可以访问（页表以及页目录项都是P=1）
bool check_vaddr_present(uint32_t vaddr){
    uint32_t * pde_ptr = get_pde(vaddr);
    uint32_t * pte_ptr;
    if(*pde_ptr==(*pde_ptr)|0x00000001){
         pte_ptr = get_pte(vaddr);
         if(*pte_ptr==(*pte_ptr)|0x00000001){
            return True;
         }
         else{
             return False;
         }
    }
    else{
        return False;
    }
}

//注意：输入的值结尾为FFF时要防止与错误输出冲突
//本函数不会无视页表的P位
//用于根据已安装的页表进行虚拟地址转换
uint32_t vmm_v2p(uint32_t vaddr){
    uint32_t pte_addr = get_pte(vaddr);
    uint32_t * ptr = (uint32_t*)pte_addr;
    if(check_vaddr_present(vaddr)){
        //可访问的vaddr
        return (*(ptr)&0xFFFFF000)|(0x00000FFF&vaddr);
    }
    else{
        //不可访问的vaddr
        return V2P_ERROR;
    }
}

static void vmm_user_init(){

}

//注意：所有的虚拟内存分配都是在内核态进行的，也就是说此时的页表是内核页表
//对用户页表操作需要输入：用户页目录表在内核页表中的虚拟地址  


/*
用户pdt pt都是在内核虚拟空间中分配的
*/


//*************for user vmm****************
#define USER_GET_VMM_ERROR  0xFFFFFFFF
static uint32_t user_get_pde(uint32_t user_pdt_vaddr,uint32_t target_vaddr){
    uint32_t times = (target_vaddr>>22)&0x000003FF;
    uint32_t pde_vaddr = user_pdt_vaddr+times*4;
    return pde_vaddr;
}


static bool vmm_user_check_pt_present(uint32_t user_pdt_vaddr,uint32_t target_vaddr){
    uint32_t times = (target_vaddr>>22)&0x000003FF;
    uint32_t pde_vaddr = user_pdt_vaddr+times*4;     //目标pde虚拟地址
    uint32_t * ptr = (uint32_t*)pde_vaddr;
    if(*ptr==(*ptr)|0x00000001){
        //表示存在present
        return True;
    }
    else{
        //表示不存在present
        return False;
    }
}


//创建的页表需要在内核中分配
static bool create_user_page_table(uint32_t pde_vaddr){
    //首先在内核中获取虚拟页
    uint32_t re_vaddr = vmm_kern_alloc();    
    if(re_vaddr==KERN_VMM_ALLOC_ERRO){
        return False;
    }
    else{
        //进行单页内存清理
        bzero(re_vaddr,PAGE_SIZE);
        uint32_t page_paddr = vmm_v2p(re_vaddr);
        //获取页表物理地址i
        if(page_paddr == V2P_ERROR){
            //转换失败！！！
            return False;
        }
        else{
        //进行用户页表映射
            uint32_t * ptr = (uint32_t *)pde_vaddr;
            *ptr = (page_paddr&0xFFFFF000)+PAGE_DESC_RW_W+PAGE_DESC_US_U+PAGE_DESC_G+PAGE_DESC_P;
        }
        return True;
    }
}

//第一个参数是需要分配的虚拟内存池对应的进程
//第二个参数是需要分配的目标地址
//默认要求此进程已经加载并且正在执行中
uint32_t vmm_user_alloc_one_page(TCB_t * tcb_ptr,uint32_t vaddr){
    if(!USER_PAGE_DIR_IS_LOADED){
        goto error_out;
    }
    bool is_kern_thread = tcb_ptr->is_kern_thread;
    if(is_kern_thread){
        goto error_out;
    }
    bitmap user_vmm_pool = tcb_ptr->user_vmm_pool;    //bitmap
    uint32_t user_pdt_vaddr = tcb_ptr->pdt_vaddr;    //pdt



    uint32_t vaddr_get=bitmap_alloc_one_page(user_vmm_pool,vaddr);
    if(vaddr_get == BITMAP_RETURN_ERRO){
        goto clean_bitmap_alloc;
    }
    else{

        if(vmm_user_check_pt_present(user_pdt_vaddr,vaddr)){
            // the page table is present,do nothing
        }
        else{
            // the page table is not present,create page table
            uint32_t times = (vaddr>>22)&0x000003FF;
            uint32_t user_dir_desc_vaddr= user_pdt_vaddr+times*4;     //目标pde虚拟地址
           if(create_user_page_table(user_dir_desc_vaddr)){
                //实际上用不了flush，因为在内核中分配时已经flush
           }
           else{
               goto clean_bitmap_alloc;
           }
        }
        pm_alloc_t phy_page = pmm_alloc_one_page();
        if(phy_page.state == 0){
            //没有可用的物理页
            goto clean_bitmap_alloc;
        }
        //获取目标页表项在内核空间中的虚拟地址
        uint32_t page_desc_vaddr = get_pte(vaddr);
        //修改页表
        //注意一点：这个page_desc_vaddr只能通过页目录最后一项找到，所以必须要用户页表加载以后才可以寻找到
        *((uint32_t*)page_desc_vaddr) = (phy_page.addr&0xFFFFF000)+PAGE_DESC_RW_W+PAGE_DESC_US_U+PAGE_DESC_G+PAGE_DESC_P;
        asm volatile ("invlpg (%0)" : : "a" (vaddr&0xFFFFF000));
        return vaddr&0xFFFFF000;
    }

    //clean以后会自动执行错误的返回
    clean_bitmap_alloc:
        bitmap_release_one_page(kern_vmm_pool,vaddr);
    error_out:
        return USER_VMM_ALLOC_ERRO;
}
uint32_t vmm_user_alloc(uint32_t user_pdt_vaddr){

}
void vmm_user_release_one_page(uint32_t target){

}

int get_user_used_vmm_info(){
    return 1;
}


//*************for user vmm****************








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