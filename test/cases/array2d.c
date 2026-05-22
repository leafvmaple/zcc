// Multi-dimensional array indexing.
int main() {
    int m[2][3];
    int i;
    int j;
    for (i = 0; i < 2; i = i + 1)
        for (j = 0; j < 3; j = j + 1)
            m[i][j] = i * 3 + j;
    printf("%d\n", m[0][0]);
    printf("%d\n", m[1][2]);
    return 0;
}
