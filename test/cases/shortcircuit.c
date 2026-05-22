int side(int x) {
    printf("call%d\n", x);
    return x;
}
int main() {
    int a = 0 && side(1);
    printf("%d\n", a);
    int b = 1 || side(2);
    printf("%d\n", b);
    printf("%d\n", !0);
    printf("%d\n", !5);
    return 0;
}
