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

void chacha20_state_init(unsigned int * state,unsigned int * key,unsigned int * nonce,unsigned int block)	{
#if 0	
	get_random_bytes(key,sizeof(unsigned int)*8);

	get_random_bytes(nonce,sizeof(unsigned int)*3);
#endif
	state[0] = 0x61707865;

	state[1] = 0x3320646e;

	state[2] = 0x79622d32;

	state[3] = 0x6b206574;

	unsigned long long int i = 4;

	while ( i < 12 )	{
		
		state[i] = reverse(key[i-4]); 
		
		i++;
	}
	
// Above while loop incorporates key to state in little endian form

	state[12] = block;
	
	i++;

	while ( i < 16 )	{
		
		state[i] = reverse(nonce[i-13]);

		i++;
	}
}

static int __init chacha20_init(void)	{
	
	unsigned int * key = (unsigned int*)kzalloc(sizeof(unsigned int)*8,GFP_KERNEL);

	unsigned long long int i = 0;

	unsigned long long int dif = 0x04050607 - 0x00010203;
	
	key[0]= 0x00010203;

	i++;

	while ( i < 16 	)	{
		
		key[i] = key[i-1] + dif;

		i++;
	}
	
	unsigned int * nonce = (unsigned int*)kzalloc(sizeof(unsigned int)*3,GFP_KERNEL);

	i = 0;

	nonce[0] = 0x00000009;

	nonce[1] = 0x0000004a;

	nonce[2] = 0x00000000;

	unsigned int * state = (unsigned int*)kzalloc(sizeof(unsigned int)*16,GFP_KERNEL);

	chacha20_state_init(state,key,nonce,1);

	i = 0;

	while ( i < 16 )	{
		
		printk(KERN_ALERT "%.4x",state[i]);

		i++;

	}
	
	kfree(state);

	kfree(key);

	kfree(nonce);

	return 0;
}

static void __exit chacha20_exit(void)	{


}

module_init(chacha20_init);

module_exit(chacha20_exit);
