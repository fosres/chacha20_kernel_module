#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x2f398466, "module_layout" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0x5a4001dd, "kernel_write" },
	{ 0x335cef9e, "kernel_read" },
	{ 0x4073ce38, "filp_close" },
	{ 0x50034bfd, "vfs_llseek" },
	{ 0x130cb5f7, "filp_open" },
	{ 0x37a0cba, "kfree" },
	{ 0x306f2e8e, "kmem_cache_alloc_trace" },
	{ 0x1b279e7d, "kmalloc_caches" },
	{ 0x7c32d0f0, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "6613A0AAACC97154DC85230");
