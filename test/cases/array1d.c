int main() {
    int a[5];
    int i;
    for (i = 0; i < 5; i = i + 1) a[i] = i * i;
    int s = 0;
    for (i = 0; i < 5; i = i + 1) s = s + a[i];
    printf("%d\n", s);
    return 0;
}
