// Global array initializer (constant aggregate) and a narrowed char global.
int g[3] = {4, 5, 6};
char gc = 321;
int main() {
    printf("%d %d %d\n", g[0], g[1], g[2]);
    printf("%d\n", gc);
    return 0;
}
