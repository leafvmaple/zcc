int main() {
    int sum = 0;
    int i;
    for (i = 1; i <= 5; i = i + 1) sum = sum + i;
    printf("%d\n", sum);
    int p = 1;
    for (int k = 1; k <= 4; k = k + 1) p = p * k;
    printf("%d\n", p);
    int n = 3;
    for (;;) {
        n = n - 1;
        if (n == 0) break;
    }
    printf("%d\n", n);
    return 0;
}
