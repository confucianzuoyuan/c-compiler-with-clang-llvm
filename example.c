int puts(char *c);
int printf(const char *format, ...);
void* malloc(unsigned long);

int main(void) {
    int* ptr = (int*)malloc(10 * sizeof(int));
    puts("Hello, world!");
    printf("hello world\r\n");
}