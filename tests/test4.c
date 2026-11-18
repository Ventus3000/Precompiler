// Test commenti e tutto -i -o

typedef unsigned int uint;
typedef float real;

//Lorem Ipsum is simply dummy text of the printing and typesetting
/*Lorem Ipsum is simply dummy text of the printing and typesetting */
/*Lorem Ipsum is simply dummy text of 
the printing and typesetting*/

uint contatore = 0, limite = 10;
real media = 0.5;
int x = 3, y;
long grande;          
char buffer[100];     
 
int main() {
  for (contatore = 0; contatore < limite; contatore++) {
    media = media + x;
  }
  printf("%f %d\n", media, y);
  return 0;
}