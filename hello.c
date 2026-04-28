void terminal_write(const char *str, int len) {
    for (int i = 0; i < len; i++) {
        *(char*)(0x10000000) = str[i];
    }
}

/* Uncomment line11 - line39
 * when implementing formatted output
 */
#include <stdlib.h>  // for itoa() and utoa()
#include <string.h>  // for strlen() and strcat()
#include <stdarg.h>  // for va_start(), va_end(), va_arg() and va_copy()

//###########################################################################
//#		  MALLOC - FREE - IMPLEMENTATION                            #
//###########################################################################

/* Uncomment line46 - line57
 * when implementing dynamic memory allocation
 */
char* test = "Test\n\r";

extern char __heap_start, __heap_end;
static char* brk = &__heap_start;
char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_end) {
        terminal_write("_sbrk: heap grows too large\r\n", 29);
        return NULL;
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}


//For aligning by 4 bytes boundary 
#define ALIGN_4(x) (((x) + 3) & ~3)
#define BLOCK_SIZE 20



typedef struct meta_block *meta_block_ptr;

// This is a meta data at front of each memory block, using this will help us to manage memory
// by managing the meta data block

struct meta_block{
	size_t mem_size;			// This contain the size of memory block after the meta block
	meta_block_ptr next;			// This contains pointer to next meta block
	meta_block_ptr prev;			// This contains pointer to previous meta block
	void* ptr;				// Used for checking tail and area that is asked to be freed 
	int free;				// if 1 mean its free 0 means occupied
	char mem_area[];			// This marks beginning of the memory block where data is actually stored
};

void* head = NULL;		//points to the first metadata

//find a memory which is free as well as greater than the size required
meta_block_ptr find_mem_block(meta_block_ptr tail, size_t mem){
	// we create a temp pointer that points to head
	meta_block_ptr temp_head = head;
	// if head is pointed to null, or till a free block with enough space is found we loop
	while(temp_head && !(temp_head->free && temp_head->mem_size >= mem)){
		// in case we want to extend block the tail will always point to last mem block
		tail = temp_head;
		temp_head = temp_head->next;
	}
	//return the block you found
	return temp_head;
}

//In case where you don't find such memory you need to extend the heap
meta_block_ptr extend_heap(meta_block_ptr tail, size_t mem){
	//you save the end of heap 
	meta_block_ptr current_heap_end = (meta_block_ptr)_sbrk(0);
	//check if we can increase the memory, the _sbrk(BLOCK_SIZE+mem) returns _sbrk(0) if successful
	if(_sbrk(BLOCK_SIZE + mem) == NULL)
		return NULL;				//if fails return NULL
	//create a new heap and add its va;ue
	meta_block_ptr new_heap = current_heap_end;
	new_heap->mem_size = mem;			//mem_size will be the memory size asked for
	new_heap->next = NULL;				//next block will be NULL
	new_heap->prev = tail;				//previous block is tail as tails should always point last metadata 
							//without the tail pointer we would have to traverse the whole list
	new_heap->ptr = new_heap->mem_area;		//The pointer points to the memory area beginning
	new_heap->free = 1;				//1 means free
	if(tail)
		tail = new_heap;			//tail will point to the new heap if its not null
	return new_heap;
}

//If we get a large enough mem we need to split it 
void split_mem_block(meta_block_ptr large_block, size_t mem){
	// The block will split into (large_block.......split_block.........)
	meta_block_ptr split_block;

	// large block will hold data hence the start of split block will be the below
	split_block = (meta_block_ptr)(large_block->mem_area + mem);

	// below shows mem size of split block
	split_block->mem_size = large_block->mem_size - mem - BLOCK_SIZE;

	// split's next block should point to large block next
	split_block->next = large_block->next;

	// we also have to change the next block's prev pointer
	if(split_block->next)
		split_block->next->prev = split_block;

	// then prev pointer of split block is large_block
	split_block->prev = large_block;

	// we save the split block ptr with mem_area start address
	split_block->ptr = split_block->mem_area;

	// split block is free
	split_block->free = 1;

	// large block next block is split block
	large_block->next = split_block;

	// its mem size will be mem asked 
	large_block->mem_size = mem;
	
}

void* my_malloc(size_t size){
	meta_block_ptr mem_needed, tail;
	size_t aligned_size = ALIGN_4(size);

	if(head){
		tail = head;
		mem_needed = find_mem_block(tail, aligned_size);
		if(mem_needed){
			if((mem_needed->mem_size - aligned_size) >= (BLOCK_SIZE + 4))
				split_mem_block(mem_needed, aligned_size);
			mem_needed->free = 0;
		}else {
			mem_needed = extend_heap(tail, aligned_size);
			if(!mem_needed)
				return NULL;
		}	

	}else{
		// Head NULL means its the first time we are using malloc
		mem_needed = extend_heap(tail, aligned_size);
		// if sbrk fails
		if(!mem_needed)
			return NULL;
		if(!head)
			head = mem_needed;
	}
	return mem_needed->mem_area;
}


//In case where side blocks are emplt we need to merge it into one when freeing
meta_block_ptr merge_block(meta_block_ptr freed_block){
	// we just need to merge the prev and next block if both are free
	if(freed_block->next && freed_block->next->free){	
		freed_block->mem_size += BLOCK_SIZE + freed_block->next->mem_size;
		freed_block->next = freed_block->next->next;
		if(freed_block->next)
			freed_block->next->prev = freed_block;	
	}
	return freed_block;
}

//getting block address when using free as free input is mem_area address
meta_block_ptr get_block(void* p){
	char* temp;
	temp = p;
	temp -= BLOCK_SIZE;
	p = temp;
	return p;
}

//cheking if valid address using ptr in mem_block
int valid_address(char* p){
	if(head){
		if(p > (char *)head && p < _sbrk(0)){
			return (p == (get_block(p)->ptr));
		}
	}
	return 0;
}

// Implementation of free
void my_free(void *p){
	meta_block_ptr free_block;
	if(valid_address(p)){
		free_block = get_block(p);
		free_block->free = 1;

		//merge with previous block if free
		if(free_block->prev && free_block->prev->free)
			free_block = merge_block(free_block->prev);
		if(free_block->next)
			merge_block(free_block);
	}else{
		terminal_write("Wrong Address!!\n\r",17);
	}
}



	

/* Implementing unsigned long long to ascii */
void ulltoa(unsigned long long num, char * BUF, int radix){
	int idx = 0;
	int rem = 0;
	while(num > 0){
		rem = num % radix;
		num = num / radix;
		if(rem <= 9){
			*(BUF + idx) = ('0' + rem);
			idx++;
		}
		else{
			*(BUF + idx) = ('A' + rem % 10);
			idx++;
		}
	}
	//reverse it
	int start = 0;
	int end = idx - 1;
	while(start < end){
		char temp = *(BUF + start);
		*(BUF + start) = *(BUF + end) ;
		*(BUF + end) = temp;
		start++;
		end--;
	}
	*(BUF + idx) = '\0';
}
		

void format_to_str(char* out, const char* fmt, va_list args) {
    for(out[0] = 0; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            strncat(out, fmt, 1);
        } else {
            fmt++;
            if (*fmt == 's') {
                strcat(out, va_arg(args, char*));
            } else if (*fmt == 'd') {
                itoa(va_arg(args, int), out + strlen(out), 10);	/* structure of itoa is (integer, char * buffer, radix) */
            } else if (*fmt == 'c') {
		//terminal_write(test, 4);
		char CBUF[2];
		CBUF[0] = va_arg(args, int);
		CBUF[1] = '\0';
		strcat(out, CBUF);
	    } else if (*fmt == 'x') {
		//terminal_write(test, 4);
		utoa(va_arg(args, int), out + strlen(out), 16);
	    } else if (*fmt == 'u') {
		//terminal_write(test, 4);
		utoa(va_arg(args, unsigned int), out + strlen(out), 10);
	    } else if (*fmt == 'p') {
		//terminal_write(test, 4);
		char * ABUF = "0x";
		strcat(out, ABUF);
		utoa(va_arg(args, unsigned int), out + strlen(out), 16);
	    } else if (strncmp(fmt,"llu",3) == 0){
		//terminal_write(test, 4);
		fmt = fmt + 2;
		ulltoa(va_arg(args, unsigned long long), out + strlen(out), 10);
		
	    }

        }
    }
}

size_t format_to_str_len(const char *fmt, va_list args){
	// I just want a simple code not exact , hence 20 bytes 
	// extra are nothing in case of integer
	unsigned int count = 0;
	while(*fmt != '%'){
		fmt++;
		count++;
	}
	fmt++;
	if(*fmt == 's')
		return count + strlen(fmt) + strlen(va_arg(args, char*));
	else
		return count + strlen(fmt) + 20;

	return 0;	
}
	
	

int printf(const char* format, ...) {
    //char buf[512];
    /* Here the va_list is a pointer that points to all arguments this is effectlively stored
     * in stack and exchanged with other function through stack */
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);
    unsigned int len = format_to_str_len(format, args_copy);
    char *buf = my_malloc(len);
    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));
    va_end(args_copy);
    my_free(buf);

    return 0;
}



int main() {
    char* msg = "Hello, World!\n\r";
    terminal_write(msg, 15);

    /* Uncomment this line of code
     * when implementing formatted output
     */
    printf("%s-%d is awesome!\n\r", "egos", 2000);
    printf("%c is character $\n\r", '$');
    printf("%c is character 0\n\r", (char)48);
    printf("%x is integer 1234 in hexadecimal\n\r", 1234);
    printf("%u is the maximum of unsigned int\n\r", (unsigned int)0xFFFFFFFF);
    printf("%p is the hexadecimal address of the hello-world string\n\r", msg);
    printf("%llu is the maximum of unsigned long long\n\r", 0xFFFFFFFFFFFFFFFFULL);
    return 0;
}
