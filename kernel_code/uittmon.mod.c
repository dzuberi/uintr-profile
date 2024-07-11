#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x92997ed8, "_printk" },
	{ 0x505889cb, "remap_pfn_range" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xc466d2c9, "current_task" },
	{ 0x55ea0f93, "find_get_pid" },
	{ 0xd0c1d157, "pid_task" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0xd6597a16, "cdev_alloc" },
	{ 0x613de5f8, "cdev_init" },
	{ 0x2c6b2811, "cdev_add" },
	{ 0xbe6cae8f, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xb0d6438a, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "65A36B1C694261AB075DF60");
