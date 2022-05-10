#include <stdio.h>

int main(int argc, char **argv)
{
  char c;

  if (argc == 2) {
    for (int i = 0; argv[1][i] != '\0'; i++) {
      c = argv[1][i];
      printf("%c", (c - i));
    }
    printf("\n");
    return (0);
  } else {
    printf("USAGE:\n\t./a.out <string to decode>");
    return (0);
  }
}
