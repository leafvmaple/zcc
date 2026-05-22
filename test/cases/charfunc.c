// char function parameters and return values truncate to one byte;
// inc(127) overflows a signed char to -128.
char inc(char c) {
    return c + 1;
}
int main() {
    char a = 65;
    printf("%c\n", inc(a));
    printf("%d\n", inc(127));
    return 0;
}
