#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/string.h>
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

unsigned int reverse(unsigned int in)	{
	
	unsigned out = 0;

	unsigned int i = 0;

	while ( i < 3 )			{
		
		out += (in & 0xff);

		out <<= 8;

		in >>= 8;

		i++;

	}

	out |= (in & 0xff);

	return out;

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

void block_function(unsigned int * state)	{
	
	unsigned long long int i = 0;
	
	unsigned int * state_orig = (unsigned int*)kzalloc(sizeof(unsigned int)*16,GFP_KERNEL);

	memcpy(state_orig,state,sizeof(unsigned int)*16);

	while ( i < 10 )	{
		
		quarteround(&state[0],&state[4],&state[8],&state[12]);

		quarteround(&state[1],&state[5],&state[9],&state[13]);

		quarteround(&state[2],&state[6],&state[10],&state[14]);

		quarteround(&state[3],&state[7],&state[11],&state[15]);
		
		quarteround(&state[0],&state[5],&state[10],&state[15]);

		quarteround(&state[1],&state[6],&state[11],&state[12]);

		quarteround(&state[2],&state[7],&state[8],&state[13]);

		quarteround(&state[3],&state[4],&state[9],&state[14]);

		i++;
	}

	i = 0;

	while ( i < 16 )	{
		
		state[i] += state_orig[i];

		i++;
	}

	kfree(state_orig);

}

void state_to_ksm(unsigned char *ksm,unsigned int * state)	{
	
	unsigned long long int i = 0;

	unsigned long long int j = 0;

	while ( i < 16 )	{
		
		memcpy(&ksm[j],&state[i],sizeof(unsigned int));

		i++; j += 4;
	}

}
 
void chacha20(const char * dest,const char *path,unsigned int * state,unsigned char * ksm_arr)	{
	
	struct file * filp = filp_open(path,O_RDONLY,S_IRUSR);

	struct file * wilp = filp_open(dest,O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);

	if (!filp)	{
		
		printk(KERN_ALERT "Error: Failed to open file");

		return;
	}

	unsigned long long int file_size = vfs_llseek(filp,0,SEEK_END);

	unsigned long long int block_i = 1;

	unsigned int * state_orig = (unsigned int*)kzalloc(sizeof(unsigned int)*16,GFP_KERNEL);

	memcpy(state_orig,state,sizeof(unsigned int)*16);

	loff_t * pos = (loff_t *)kzalloc(sizeof(loff_t),GFP_KERNEL);

	loff_t * outset = (loff_t *)kzalloc(sizeof(loff_t),GFP_KERNEL);

	unsigned char * arr = (unsigned char *)kzalloc(sizeof(unsigned char)*64,GFP_KERNEL);

	unsigned long long int i = 0, n = 0;
	
	while ( n < file_size )	{
		
		memset(state,0x0,sizeof(unsigned int)*16);

		memcpy(state,state_orig,sizeof(unsigned int)*16);

		state[12] = block_i;

		block_function(state);

		memset(ksm_arr,0x0,sizeof(unsigned char)*64);

		state_to_ksm(ksm_arr,state);

		i = 0;

		printk(KERN_ALERT "Keystream for block %llu:",block_i);

		while ( i < 64 )		{
			
			printk(KERN_CONT "%.2x:",ksm_arr[i]);

			i++;
		}

		kernel_read(filp,arr,sizeof(unsigned char)*64,pos);

		i = 0;

		while ( ( i < 64 ) && ( n < file_size) )	{
			
			arr[i] ^= ksm_arr[i];

			kernel_write(wilp,&arr[i],sizeof(unsigned char),outset);

			i++;n++;
		}
	
		block_i++;

	}
	
	filp_close(filp,NULL);

	filp_close(wilp,NULL);

	kfree(state_orig);

	kfree(pos);

	kfree(outset);

	kfree(arr);
}

const unsigned char * hextable[16] = 

{
	"0000","0001","0010","0011",
	
	"0100","0101","0110","0111",

	"1000","1001","1010","1011",

	"1100","1101","1110","1111"

};

void reverse_string(unsigned char *s,unsigned long long int n)	{
	
	unsigned char temp = 0;

	unsigned long long int i = 0, j = n-1;

	while ( i < j )		{
		
		temp = s[i]; s[i] = s[j];

		s[j] = temp; i++; j--;

	}

}

// size of key is 8 32-bit unsigned integers; out is 256 bytes


void key_to_bitarr(unsigned char * out,unsigned int * key)	{
	
	unsigned long long int i = 0, j = 0; 

	unsigned int kval = 0;

	unsigned int * k_p = &key[7];

	unsigned char * o_p = out;

	unsigned char temp = 0;

	unsigned char x[5] = {0};

	while ( k_p >= key )				{
		
		kval = *k_p;

		j = 0;

		while ( j < 8 )		{
			
			temp = kval & 0xf;

			memcpy(x,hextable[temp],sizeof(unsigned char)*4);

			printk(KERN_ALERT "Before reverse:%s",x);

			reverse_string(x,4);
			
			printk(KERN_ALERT "After reverse:%s",x);

			memcpy(o_p,x,sizeof(unsigned char)*4);

			o_p += 4;

			kval >>= 4;

			j++;
		}

		k_p--;
	}

}


static int __init chacha20_init(void)	{
	
	unsigned int * key = (unsigned int*)kzalloc(sizeof(unsigned int)*8,GFP_KERNEL);
	
	unsigned long long int i = 0;

	unsigned long long int dif = 0x04050607 - 0x00010203;
	
	key[0]= 0x00010203;

	i++;

	while ( i < 8 	)	{
		
		key[i] = key[i-1] + dif;

		i++;
	}

	i = 0;
	
	printk(KERN_ALERT "Printing contents of key:");

	while ( i < 8 )	{
		
		printk(KERN_ALERT "%.4x",key[i]);

		i++;
	}
	
	unsigned int * nonce = (unsigned int*)kzalloc(sizeof(unsigned int)*3,GFP_KERNEL);

	i = 0;

	nonce[0] = 0x00000000;

	nonce[1] = 0x0000004a;

	nonce[2] = 0x00000000;

	unsigned int * state = (unsigned int*)kzalloc(sizeof(unsigned int)*16,GFP_KERNEL);
	
	unsigned int * state_orig = (unsigned int*)kzalloc(sizeof(unsigned int)*16,GFP_KERNEL);

	memcpy(state_orig,state,sizeof(unsigned int)*16);

	chacha20_state_init(state,key,nonce,1);
	
	unsigned char * ksm = (unsigned char*)kzalloc(sizeof(unsigned char)*64,GFP_KERNEL);

	chacha20("test.out.txt","test.txt",state,ksm);

	memset(state,0x0,sizeof(unsigned int)*16);

	memcpy(state,state_orig,sizeof(unsigned int)*16);

	chacha20_state_init(state,key,nonce,1);
	
	chacha20("test.out.txt.recover","test.out.txt",state,ksm);

	unsigned char x[8] = {0};

	memcpy(x,hextable[14],sizeof(unsigned char)*4);

	printk(KERN_ALERT "x:%s",x);

	unsigned char * bitarr = (unsigned char*)kzalloc(sizeof(unsigned char)*256,GFP_KERNEL);
	
	key_to_bitarr(bitarr,key);

	i = 0; 
	
	printk(KERN_ALERT "Contents of bitarr:");

	while ( i < 256 )	{
		

		if ( (i % 8 == 0) && (i != 0) )	{
			
			printk(KERN_CONT "%c",'|');	

		}
		
		printk(KERN_CONT "%c",bitarr[i]);

		i++;

	}

	i = 0;
	
	kfree(bitarr);

	kfree(ksm);

	kfree(state);

	kfree(state_orig);

	kfree(key);

	kfree(nonce);

	return 0;
}

static void __exit chacha20_exit(void)	{


}

module_init(chacha20_init);

module_exit(chacha20_exit);
