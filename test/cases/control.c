int main() {
    int x = 0;
    int count = 0;
    while (x < 20) {
        x = x + 1;
        if (x % 2 == 0) continue;
        if (x > 10) break;
        count = count + 1;
    }
    printf("%d\n", count);
    int v = 42;
    if (v > 40) printf("big\n"); else printf("small\n");
    return 0;
}
