#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable_types.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
//------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HCMUS");
/*MY sys_call_table address*/
//ffffffff820013a0 R sys_call_table
void** system_call_table_addr = (void**)0xffffffff81600340;
//custom syscall that takes process name
asmlinkage long (*ref_sys_open)(const char __user* filename, int flags, umode_t mode);
asmlinkage long (*ref_sys_write)(unsigned int fd, const char __user* buf, size_t count);
asmlinkage long new_sys_open(const char __user* filename, int flags, umode_t mode) {
    printk(KERN_INFO "[OPENHOOK]: Opening file name %s", filename);
    return ref_sys_open(filename, flags, mode);
}
asmlinkage long new_sys_write(unsigned int fd, const char __user* buf, size_t count) {
    printk(KERN_INFO "[WRITEHOOK]: Writing on file name %s", buf);
    printk(KERN_INFO "[WRITEHOOK]: Number of bytes in file name: %zu", count);
    return ref_sys_write(fd, buf, count);
}
/*Make page writeable*/
int make_rw(unsigned long address) {
    unsigned int level;
    pte_t* pte = lookup_address(address, &level);
    if (pte->pte & ~_PAGE_RW) {
        pte->pte |= _PAGE_RW;
    }
    return 0;
}
/* Make the page write protected */
int make_ro(unsigned long address) {
    unsigned int level;
    pte_t* pte = lookup_address(address, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
    return 0;
}
static int __init entry_point(void) {
    printk(KERN_INFO "[OPENWRITEHOOK]: Hook is loading...\n");
    /*MY sys_call_table address*/
    /* Replace custom syscall with the correct system call name (write,open,etc) to hook*/
    ref_sys_open = system_call_table_addr[__NR_open];
    ref_sys_write = system_call_table_addr[__NR_write];
    printk("Origin syscall was stored...");
    /*Disable page protection*/
    make_rw((unsigned long)system_call_table_addr);
    /*Change syscall to our syscall function*/
    system_call_table_addr[__NR_open] = new_sys_open;
    system_call_table_addr[__NR_write] = new_sys_write;
    printk(KERN_INFO "[OPENWRITEHOOK]: Hook was successfully loaded!!\n");
    return 0;
}
static void __exit exit_point(void) {
    printk(KERN_INFO "[OPENWRITEHOOK]: Hook was successfully unloaded\n");
    /*Restore original system call */
    system_call_table_addr[__NR_open] = ref_sys_open;
    system_call_table_addr[__NR_write] = ref_sys_write;
    /*Renable page protection*/
    make_ro((unsigned long)system_call_table_addr);
}
module_init(entry_point);
module_exit(exit_point);