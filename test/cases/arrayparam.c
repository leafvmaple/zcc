// Passing an array to a function and indexing the decayed pointer parameter.
int sum(int a[], int n) {
    int s = 0;
    int i;
    for (i = 0; i < n; i = i + 1) s = s + a[i];
    return s;
}
int main() {
    int x[4];
    int i;
    for (i = 0; i < 4; i = i + 1) x[i] = i + 1;
    printf("%d\n", sum(x, 4));
    return 0;
}
