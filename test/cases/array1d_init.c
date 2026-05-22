// Local array initializers, with and without trailing zero-fill.
int main() {
    int a[3] = {1, 2, 3};
    int b[5] = {7, 8};
    printf("%d %d %d\n", a[0], a[1], a[2]);
    printf("%d %d %d\n", b[0], b[1], b[4]);
    return 0;
}
