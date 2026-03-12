int fact(int n)
{
    if (n <= 1)
        return 1;
    return n * fact(n - 1);
}

int fib(int n)
{
    if (n <= 1)
        return n;
    int a = 0, b = 1;
    int i;
    for (i = 2; i <= n; i = i + 1) {
        int t = a + b;
        a = b;
        b = t;
    }
    return b;
}

int main()
{
    int x = fact(10);
    int y = fib(10);
    printf("%d %d\n", x, y);
    return 0;
}