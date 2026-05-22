// char is an 8-bit type: assignment truncates to one byte.
int main() {
    char c;
    c = 321;
    printf("%d\n", c);
    char d;
    d = 65;
    printf("%c\n", d);
    return 0;
}
