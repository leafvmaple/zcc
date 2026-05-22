// char arrays are stored as 8-bit elements and indexed correctly.
int main() {
    char s[4];
    s[0] = 72;
    s[1] = 105;
    s[2] = 33;
    s[3] = 0;
    int i;
    for (i = 0; i < 3; i = i + 1) printf("%c", s[i]);
    printf("\n");
    return 0;
}
