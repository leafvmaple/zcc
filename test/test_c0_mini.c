// Minimal C0 feature tests

int main() {
    // Test 1: for with declaration init + assignment step
    int sum = 0;
    for (int i = 1; i <= 5; i = i + 1) {
        sum = sum + i;
    }
    // sum = 15
    printf("sum=%d\n", sum);

    // Test 2: for with expression init + expression step
    int x = 0;
    for (x = 1; x < 4; x = x + 1) {
    }
    // x = 4
    printf("x=%d\n", x);

    // Test 3: char
    char c = 'Z';
    printf("c=%c\n", c);

    // Test 4: empty for parts
    int n = 3;
    for (;;) {
        n = n - 1;
        if (n == 0) break;
    }
    printf("n=%d\n", n);

    return 0;
}
