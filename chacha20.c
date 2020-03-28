#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
MODULE_LICENSE("Dual BSD/GPL");
MODULE_INFO(intree,"Y");

unsigned int bit_count(unsigned int shift)	{
	
	if ( shift >= 32 )	{
		
		printk(KERN_ALERT "Error: Shift is greater than result size!\n");

		return -1;
	}

	unsigned int output = 0x0;

	do 					{
		output |= 0b1;

		if ( shift > 1 )	{
			
			output <<= 1;
		}

		shift--;

	} while (shift > 0 );

	return output;

}

unsigned int rotate_left(unsigned int input,unsigned int shift)	{
	
	unsigned int extract = bit_count(shift) << (32 - shift);

	extract = input & extract;

	extract = ( extract >> (32 - shift ) );

	return ((input << shift ) | extract);

}

void quarteround(unsigned int *a,unsigned int *b,unsigned int *c,unsigned int *d)	{

	*a += *b; *d ^= *a; *d = rotate_left(*d,16);

	*c += *d; *b ^= *c; *b = rotate_left(*b,12);

	*a += *b; *d ^= *a; *d = rotate_left(*d,8);

	*c += *d; *b ^= *c; *b = rotate_left(*b,7);
}

static int __init chacha20_init(void)	{
	
	unsigned int x = 0x7998bfda;

	unsigned int y = rotate_left(x,7);

	printk(KERN_ALERT "%.4x",y);
	
	return 0;
}

static void __exit chacha20_exit(void)	{


}

module_init(chacha20_init);

module_exit(chacha20_exit);
