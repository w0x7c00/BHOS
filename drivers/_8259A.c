#include "port.h"
#include "types.h"
static uint16_t m_control_port = 0x20;     //主片控制端口号  main
static uint16_t m_data_port = 0x21;	       //从片数据端口号
static uint16_t o_control_port = 0xA0;     //从片控制端口号  other
static uint16_t o_data_port = 0xA1;        //从片数据端口号

void _8259A_init(){
	//主片初始化
	outb(m_control_port,0x11);
	outb(m_data_port,0x20);
	outb(m_data_port,0x04);
	outb(m_data_port,0x01);
	//从片初始化
	outb(o_control_port,0x11);
	outb(o_data_port,0x28);
	outb(o_data_port,0x02);
	outb(o_data_port,0x01);
}