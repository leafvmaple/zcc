int g = 10;
const int K = 5;
int add(int x) {
    return x + g + K;
}
int main() {
    printf("%d\n", add(1));
    g = 20;
    printf("%d\n", add(1));
    return 0;
}
