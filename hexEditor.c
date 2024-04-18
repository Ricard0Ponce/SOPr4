#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <curses.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Variable global para mejor legibilidad */
int fd; // Archivo a leer

// Aqui se mapea la forma en la que se muestran los valores
char *hazLinea(char *base, int dir)
{
  char linea[100]; // La linea es mas pequeña
  int o = 0;
  // Muestra 16 caracteres por cada linea
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
  long fs = st.st_size;

  char *map = mmap(0, fs, PROT_READ, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED)
  {
    close(fd);
    perror("Error mapeando el archivo");
    return (NULL);
  }

  return map;
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

  int col;
  int ren;
  col = 9;
  ren = 0;
  move(ren, col);
  // getchar(); // NEA
  int c = getch();
  while (c != 26)
  {
    switch (c)
    {
    case KEY_LEFT:
      if (col == 0)
      {
        col = 0;
      }
      else
      {
        col -= 3;
      }

      break;
    case KEY_RIGHT:
      if (col == 72)
      {
        col = 0;
      }
      else
      {
        col += 3;
      }
      break;
    case KEY_DOWN:
      if (ren < 24)
      {
        ren++;
      }
      else
      {
        map += 16; // Mueve el puntero al siguiente bloque de 16 renglones
        clear();
        for (int i = 0; i < 25; i++)
        {
          char *l = hazLinea(map, i * 16);
          move(i, 0);
          addstr(l);
        }
        ren = 24; // Reinicia la posición del renglón en 24
      }
      break;
    case KEY_UP:
      if (ren == 0) // Cuando estas en la posicion inicial y vas hacia arriba se rompe.
      {
        if (map > 0) // Verifica que no estés en el inicio del archivo
        {
          map -= 16; // Mueve el puntero al bloque anterior de 16 renglones
          clear();
          for (int i = 0; i < 25; i++)
          {
            char *l = hazLinea(map, i * 16);
            move(i, 0);
            addstr(l);
          }
          ren = 0; // Reinicia la posición del renglón en 0
        }
      }
      else if (ren < 25)
      {
        ren--;
      }
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
