#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/kallsyms.h>
#include <linux/fcntl.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

#include <asm/cacheflush.h>
#include <asm/unistd.h>
#include <asm/pgtable_types.h>

MODULE_LICENSE("GPL");

/*sys_call_table address*/
static void **SYS_CALL_TABLE;		

static asmlinkage long (*original_syscallopen) (const char __user *, int, umode_t);
static asmlinkage long (*original_syscallwrite) (unsigned int, const char __user *, size_t);

static asmlinkage long hook_open(const char __user *filename, int flags, umode_t mode){
	char *file;
	printk(KERN_INFO "[DEBUG HOOK] OPEN HOOKED HERE.\n");
	file   = kmalloc(256, GFP_KERNEL);
   	copy_from_user(file, filename, 256);
	printk(KERN_INFO "[OPEN HOOK] Process %s.\n", current->comm);
	printk(KERN_INFO "[OPEN HOOK] Opens file %s.\n", file);
	kfree(file);
	return original_syscallopen(filename, flags, mode);
}


static asmlinkage long hook_write(unsigned int fildes, const char __user *buffer, size_t count){
	char *kbuf, *filename;
	kbuf   		= kmalloc(256, GFP_KERNEL);
	copy_from_user(kbuf, buffer, 256);
	// char *d_path(const struct path * path, char * buf,int buflen);
	filename	= d_path(&fcheck_files(current->files, fildes)->f_path, kbuf, 256);
	printk(KERN_INFO "[DEBUG HOOK] WRITE HOOKED HERE\n");
 	printk(KERN_INFO "[WRITE HOOK] Process %s.\n", current->comm);
	printk(KERN_INFO "[WRITE HOOK] Writes %zu bytes\n.", count);
	printk(KERN_INFO "[WRITE HOOK] To file %s.\n", filename);
	kfree(kbuf);
	kfree(filename);
	return original_syscallwrite(fildes, buffer, count);
}


// Make writeable 
static int make_rw(unsigned long address) {
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if (pte->pte &~_PAGE_RW) {
		pte->pte |= _PAGE_RW;
	}
	return 0;
}

// Make write protected 
static int make_ro(unsigned long address) {
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~_PAGE_RW;
	return 0;
}

static int __init entry_point(void){
	// Gets Syscall Table **
	printk(KERN_INFO "[DEBUG HOOK] Captain Hook loaded successfully..\n");
 	SYS_CALL_TABLE = (void*)kallsyms_lookup_name("sys_call_table");

	printk(KERN_INFO "[DEBUG HOOK] Hooks Will Be Set.\n");
	printk(KERN_INFO "[DEBUG HOOK] System call table at %p\n", SYS_CALL_TABLE);

	/* Replace custom syscall with the correct system call name (write,open,etc) to hook*/
	original_syscallopen = SYS_CALL_TABLE[__NR_open];
	original_syscallwrite = SYS_CALL_TABLE[__NR_write];

	printk(KERN_INFO "[DEBUG HOOK] Disable page protection success.\n");
	
	// Replaces Pointer Of Syscall_read on our syscall.
	make_rw((unsigned long)SYS_CALL_TABLE);
	SYS_CALL_TABLE[__NR_open] = hook_open;
	SYS_CALL_TABLE[__NR_write] = hook_write;

	
	printk(KERN_INFO "[DEBUG HOOK] Overriding syscall open success.\n");
	printk(KERN_INFO "[DEBUG HOOK] Overriding syscall write success.\n");
	printk(KERN_INFO "[DEBUG HOOK] Overriding syscall read success.\n");

	return 0;
}

static void __exit exit_point(void){
	// Clean up our Hooks
	printk(KERN_INFO "[DEBUG HOOK] Unloaded Captain Hook successfully.\n");

	/*Restore original system call */

	SYS_CALL_TABLE[__NR_open] = original_syscallopen;
	SYS_CALL_TABLE[__NR_write] = original_syscallwrite;

	make_ro((unsigned long)SYS_CALL_TABLE);

	printk(KERN_INFO "[DEBUG HOOK] Restore syscall open success.\n");
	printk(KERN_INFO "[DEBUG HOOK] Restore syscall write success.\n");
	printk(KERN_INFO "[DEBUG HOOK] Restore syscall read success.\n");
	printk(KERN_INFO "[DEBUG HOOK] Enable page protection success.\n");
}

module_init(entry_point);
module_exit(exit_point);


MODULE_AUTHOR("Nhut-Nam Le");
MODULE_DESCRIPTION("Operating System Project 03");
MODULE_VERSION("1.0.0");

