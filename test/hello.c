int f(int arr[][3]) {
  return arr[1][2];
}

int main() {
  int arr[2][3] = {1, 2};
  return f(arr);
}