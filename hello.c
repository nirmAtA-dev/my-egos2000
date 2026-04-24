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
		
char* test = "Test";

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
		char ABUF[2];
		ABUF[0] = '0';
		ABUF[1] = 'x';
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

int printf(const char* format, ...) {
    char buf[512];
    /* Here the va_list is a pointer that points to all arguments this is effectlively stored
     * in stack and exchanged with other function through stack */
    va_list args;
    va_start(args, format);
    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));

    return 0;
}

/* Uncomment line46 - line57
 * when implementing dynamic memory allocation
 */
/*
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
*/

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
