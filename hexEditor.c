#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <curses.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
/*
Nombre: Ricardo Antonio Ponce Garcia.
Matricula: 2193052801.
Fecha: 25/04/2024.
*/

/* Variable global para mejor legibilidad */
int fd; // Archivo a leer
long fs;
long ultimoElemento = 0;
// Aqui se mapea la forma en la que se muestran los valores
char *hazLinea(char *base, int dir)
{
  char linea[200]; // La linea es mas pequeña
  int o = 0;
  // Calcula el offset en hexadecimal dinámicamente
  o += sprintf(linea, "%08x ", dir); // offset en hexadecimal
  // El siguiente for muestra la cantidad de elementos que se muestran en renglones
  for (int i = 0; i < 4; i++)
  {
    unsigned char a, b, c, d;
    a = base[dir + 4 * i + 0];
    b = base[dir + 4 * i + 1];
    c = base[dir + 4 * i + 2];
    d = base[dir + 4 * i + 3];
    o += sprintf(&linea[o], "%02x %02x %02x %02x ", a, b, c, d);
  }
  // Muestra la cantidad de columnas
  for (int i = 0; i < 16; i++)
  {
    if (isprint(base[dir + i]))
    {
      o += sprintf(&linea[o], "%c", base[dir + i]);
    }
    else
    {
      o += sprintf(&linea[o], ".");
    }
  }
  sprintf(&linea[o], "\n");
  sprintf(&linea[o], "\n\n\n\nMenu: Primer Elemento: [Ctrl + A]  Ultimo elemento: [Ctrl + B]  Salir: [Ctrl + C]  Buscar: [Ctrl +D] \n"); // Agrega "Hola" al final
  return (strdup(linea));
}

char *mapFile(char *filePath)
{
  /* Abre archivo */
  fd = open(filePath, O_RDONLY);
  if (fd == -1)
  {
    perror("Error abriendo el archivo");
    return (NULL);
  }

  /* Mapea archivo */
  struct stat st;
  fstat(fd, &st);
  fs = st.st_size;

  char *map = mmap(0, fs, PROT_READ, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED)
  {
    close(fd);
    perror("Error mapeando el archivo");
    return (NULL);
  }

  return map;
}

// Función que se ejecutará cuando se reciba la señal SIGINT
void cierre(int sig)
{
  endwin(); // Finaliza el modo ncurses
  exit(0);
}

int leeChar()
{
  int chars[5];
  int ch, i = 0;
  nodelay(stdscr, TRUE);
  while ((ch = getch()) == ERR)
    ; /* Espera activa */
  ungetch(ch);
  while ((ch = getch()) != ERR)
  {
    chars[i++] = ch;
  }
  /* convierte a numero con todo lo leido */
  int res = 0;
  for (int j = 0; j < i; j++)
  {
    res <<= 8;
    res |= chars[j];
  }
  return res;
}

int edita(char *filename)
{

  /* Limpia pantalla */
  clear();

  /* Lee archivo */
  char *map = mapFile(filename);
  if (map == NULL)
  {
    exit(EXIT_FAILURE);
  }
  // Este for define cuantos elementos hay
  for (int i = 0; i < 25; i++)
  {
    // Haz linea, base y offset
    char *l = hazLinea(map, i * 16);
    move(i, 0);
    addstr(l);
  }
  refresh();
  signal(SIGINT, cierre);
  int col;
  int ren;
  col = 9;
  ren = 0;
  int aux = 0;
  int aux2 = 1;
  /* Mapea archivo */
  struct stat st;
  fstat(fd, &st);

  long tope = st.st_size;
  move(ren, col);
  int c = getch();
  int contador = 0;
  while (c != 26)
  {
    switch (c)
    {
    case KEY_LEFT:
      if (col > 0)
      {
        col -= 3;
      }
      break;
    case KEY_RIGHT:
      if (col < 72)
      {
        col += 3;
      }
      else
      {
        col = 0; // Permite que cuando llegue al final se regrese a cero
      }
      break;
    case KEY_DOWN:
      if (tope != 0)
      {
        tope--;
        // contador++;
        if (ren < 24)
        {
          contador++;
          ren++;
        }
        else
        {
          contador++;
          aux += 16; // Mueve el puntero al siguiente bloque de 16 renglones
          clear();
          // aux++;
          for (int i = 0; i < 25; i++)
          {
            char *l = hazLinea(map, aux + i * 16); // aux2;
            move(i, 0);                            // move(i-aux, 0);
            addstr(l);
          }
          // aux++;
          // aux2=0;
          // ren = 24; // Reinicia la posición del renglón en 24
        }
      }
      break;
    case KEY_UP:
      if (contador != 0)
      {
        tope++;
        if (ren > 0)
        {
          ren--;
          contador--;
        }
        else if (map > 0) // Verifica que no estés en el inicio del archivo
        {
          aux -= 16; // Mueve el puntero al bloque anterior de 16 renglones
          clear();
          for (int i = 0; i < 25; i++)
          {
            char *l = hazLinea(map, aux + i * 16);
            move(i, 0);
            addstr(l);
          }
          contador--;
          ren = 0; // Reinicia la posición del renglón en 0 solo si se pudo mover el puntero
        }
      } // El ultimo elemento siempre es: 1474160 para Disk1.DSK
      break;
    case 1:                    /*CTRL + A */
      map = mapFile(filename); // Usar el puntero map existente, no crear uno nuevo
      if (map == NULL)
      {
        exit(EXIT_FAILURE);
      }
      clear();
      // Este for define cuantos elementos hay
      for (int i = 0; i < 25; i++)
      {
        // aux = contador + i;
        //  Haz linea, base y offset
        char *l = hazLinea(map, i * 16); // Antes i * 16
        move(i, 0);
        addstr(l);
      }
      refresh();
      contador = 0;
      ren = 0; // Reinicia la posición del renglón en 0 solo si se pudo mover el puntero
      col = 9;
      tope = st.st_size;
      aux = 0;
      break;
    case 2: /*CTRL + B */
      tope = 0;
      // Calcula la posición del último bloque de 16 renglones
      ultimoElemento = fs - (25 * 16);
      // aux = tope;
      clear();
      if (ultimoElemento < 0)
      {
        ultimoElemento = 0;
      }
      map = mapFile(filename) + ultimoElemento; // Actualiza el puntero map al último bloque
      // aux = mapFile(filename) + ultimoElemento;
      aux = 0;
      contador = st.st_size;
      aux = (int)(ultimoElemento / 16) - 25;
      for (int i = 0; i < 25; i++)
      {
        char *l = hazLinea(map, aux + i * 16);
        move(i, 0);
        addstr(l);
      }
      refresh();
      // contador = st.st_size;
      ren = 24;
      col = 9;
      break;
    case 3: // CTRL + C
      cierre(SIGINT);
      break;
    }
    move(ren, col);
    c = getch();
  }

  if (munmap(map, fd) == -1)
  {
    perror("Error al desmapear");
  }
  close(fd);
  // Para salir desde la consola alt f4
  return 0;
}

int main(int argc, char const *argv[])
{
  initscr();
  raw();
  keypad(stdscr, TRUE); /* Para obtener F1,F2,.. */
  noecho();

  /* El archivo se da como parametro */
  if (argc != 2)
  {
    printf("Se usa %s <archivo> \n", argv[0]);
    return (-1);
  }
  edita((char *)argv[1]);
  endwin();

  return 0;
}

// hi =  *(unsigned char *) &map(0x1BE + i*16+1); ///si ci
// Tamaño de un sector = 512
// Dame la direccion (HEX) -> Ctrl + G
// Primero obtener la direccion de las particiones.
//

/*
struct mbr {
  unsigned char hi, si, ci, ts,hf,sf,cf;
  unsigned int lba, tamS;
} mbr;
ts = tipo del sistema
*/

// Usamos memcpy
/*
memcpy(&mbr, &map[0x1BE+i*16],sizeof(mbr));
*/