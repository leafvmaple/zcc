void f1(int arr[]) {}

void f2(int arr[][8 + 2]) {
  f1(arr[0]);
}

int main() {
  return 0;
}