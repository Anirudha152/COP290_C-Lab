#include<stdio.h>
#include<stdarg.h>

void print_num(int count, ...) {
	va_list args;
	va_start (args, count);

	for(int i=0; i<count; i++){
		printf("%d ", va_arg(args, int));
	}
	va_end (args);
}
int main(){
	print_num(5, 4, 5, 6, 8, 7);
}
