// Zero errori -i

int a = 5;
float b = 2.5;
double somma;
char lettera = 'x';
int contatore = 0;
 
int main() {
  somma = a + b;
  for (contatore = 0; contatore < 3; contatore++) {
    printf("%c %f\n", lettera, somma);
  }
  return 0;
}