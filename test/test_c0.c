// C0 test case: tests char type, for loop, printf, scanf

int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

int main() {
    // Test for loop
    int sum = 0;
    for (int i = 1; i <= 10; i = i + 1) {
        sum = sum + i;
    }
    // sum should be 55
    printf("sum = %d\n", sum);

    // Test char
    char ch = 'A';
    printf("char = %c\n", ch);

    // Test nested for
    int total = 0;
    for (int i = 0; i < 3; i = i + 1) {
        for (int j = 0; j < 3; j = j + 1) {
            total = total + 1;
        }
    }
    printf("total = %d\n", total);

    // Test gcd
    int g = gcd(12, 8);
    printf("gcd(12,8) = %d\n", g);

    // Test while with break/continue
    int x = 0;
    int count = 0;
    while (x < 20) {
        x = x + 1;
        if (x % 2 == 0) continue;
        if (x > 10) break;
        count = count + 1;
    }
    printf("count = %d\n", count);

    // Test if-else
    int val = 42;
    if (val > 40) {
        printf("big\n");
    } else {
        printf("small\n");
    }

    return 0;
}
