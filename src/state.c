#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

// Definiciones de funciones de ayuda.
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);
/* Tarea 1 */
game_state_t* create_default_state() {
  game_state_t* state = malloc(sizeof(game_state_t));  // Estado en el heap
  if (state == NULL) return NULL;

  state->num_rows = 18;
  state->board = malloc(sizeof(char*) * state->num_rows);
  if (state->board == NULL) {
      free(state);
      return NULL;
  }

  char* rows[] = {
      "####################\n",
      "#                  #\n",
      "# d>D    *         #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "#                  #\n",
      "####################\n"
  };

  for (unsigned int i = 0; i < state->num_rows; i++) {
      size_t len = strlen(rows[i]);
      state->board[i] = malloc(len + 1);  // +1 para '\0'
      if (state->board[i] == NULL) {
          // Liberar memoria en caso de error
          for (unsigned int j = 0; j < i; j++) {
              free(state->board[j]);
          }
          free(state->board);
          free(state);
          return NULL;
      }
      strcpy(state->board[i], rows[i]);
  }

  state->num_snakes = 1;
  state->snakes = malloc(sizeof(snake_t));
  if (state->snakes == NULL) {
      for (unsigned int i = 0; i < state->num_rows; i++) {
          free(state->board[i]);
      }
      free(state->board);
      free(state);
      return NULL;
  }

  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;

  return state;
}

/*Tarea 1.1*/
game_state_t* create_amongus_state() {
  game_state_t* state = malloc(sizeof(game_state_t));
  if (state == NULL) return NULL;

  state->num_rows = 11;
  state->board = malloc(sizeof(char*) * state->num_rows);
  if (state->board == NULL) {
    free(state);
    return NULL;
  }

  // Todas las filas tienen la misma longitud: 21 columnas + \n
  char* rows[] = {
    "##############       \n",
    "#            #######\n",
    "#####             ##\n",
    "#   #             ##\n",
    "#####             ######\n",
    "#                 ##   #\n",
    "#                 ######\n",
    "#                 ##    \n",
    "#                  #    \n",
    "#      #####       #    \n",
    "########   #########    \n"
   
  };

  for (unsigned int i = 0; i < state->num_rows; i++) {
    size_t len = strlen(rows[i]);
    state->board[i] = malloc(len + 1);
    if (state->board[i] == NULL) {
      for (unsigned int j = 0; j < i; j++) free(state->board[j]);
      free(state->board);
      free(state);
      return NULL;
    }
    strcpy(state->board[i], rows[i]);
  }

  state->num_snakes = 1;
  state->snakes = malloc(sizeof(snake_t));
  if (state->snakes == NULL) {
    for (unsigned int i = 0; i < state->num_rows; i++) free(state->board[i]);
    free(state->board);
    free(state);
    return NULL;
  }

  // Posicionar snake
  state->snakes[0].tail_row = 1;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 1;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;

  set_board_at(state, 1, 2, 'd');
  set_board_at(state, 1, 3, '>');
  set_board_at(state, 1, 4, 'D');

  // Posicionar comida (fruta) en una celda segura vacía
  set_board_at(state, 5, 5, '*');

  return state;
}


/* Tarea 2 */
void free_state(game_state_t* state) {
  if (state == NULL) return;

  // Liberar cada fila del tablero
  if (state->board != NULL) {
      for (unsigned int i = 0; i < state->num_rows; i++) {
          free(state->board[i]);
      }
      free(state->board);
  }

  // Liberar las serpientes
  if (state->snakes != NULL) {
      free(state->snakes);
  }

  // Finalmente liberar el estado
  free(state);
}

/* Tarea 3 */
void print_board(game_state_t* state, FILE* fp) {
  if (state == NULL || state->board == NULL || fp == NULL) return;

  for (unsigned int i = 0; i < state->num_rows; i++) {
      fprintf(fp, "%s", state->board[i]);
  }
}


/**
 * Guarda el estado actual a un archivo. No modifica el objeto/struct state.
 * (ya implementada para que la utilicen)
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Tarea 4.1 */
/**
 * Funcion de ayuda que obtiene un caracter del tablero dado una fila y columna
 * (ya implementado para ustedes).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}


/**
 * Funcion de ayuda que actualiza un caracter del tablero dado una fila, columna y
 * un caracter.
 * (ya implementado para ustedes).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}


/**
 * Retorna true si la variable c es parte de la cola de una snake.
 * La cola de una snake consiste de los caracteres: "wasd"
 * Retorna false de lo contrario.
*/
static bool is_tail(char c) {
  return (c == 'w' || c == 'a' || c == 's' || c == 'd');
}


/**
 * Retorna true si la variable c es parte de la cabeza de una snake.
 * La cabeza de una snake consiste de los caracteres: "WASDx"
 * Retorna false de lo contrario.
*/
static bool is_head(char c) {
  return (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x');
}


/**
 * Retorna true si la variable c es parte de una snake.
 * Una snake consiste de los siguientes caracteres: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return (c == 'w' || c == 'a' || c == 's' || c == 'd' ||
          c == '^' || c == '<' || c == 'v' || c == '>' ||
          c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x');
}


/**
 * Convierte un caracter del cuerpo de una snake ("^<v>")
 * al caracter que correspondiente de la cola de una
 * snake ("wasd").
*/
static char body_to_tail(char c) {
  if (c == '^') return 'w';
  if (c == '<') return 'a';
  if (c == 'v') return 's';
  if (c == '>') return 'd';
  return '?';  // indefinido
}


/**
 * Convierte un caracter de la cabeza de una snake ("WASD")
 * al caracter correspondiente del cuerpo de una snake
 * ("^<v>").
*/
static char head_to_body(char c) {
  if (c == 'W') return '^';
  if (c == 'A') return '<';
  if (c == 'S') return 'v';
  if (c == 'D') return '>';
  return '?';  // indefinido
}


/**
 * Retorna cur_row + 1 si la variable c es 'v', 's' o 'S'.
 * Retorna cur_row - 1 si la variable c es '^', 'w' o 'W'.
 * Retorna cur_row de lo contrario
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') return cur_row + 1;
  if (c == '^' || c == 'w' || c == 'W') return cur_row - 1;
  return cur_row;
}


/**
 * Retorna cur_col + 1 si la variable c es '>' or 'd' or 'D'.
 * Retorna cur_col - 1 si la variable c es '<' or 'a' or 'A'.
 * Retorna cur_col de lo contrario
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') return cur_col + 1;
  if (c == '<' || c == 'a' || c == 'A') return cur_col - 1;
  return cur_col;
}


/**
 * Tarea 4.2
 *
 * Funcion de ayuda para update_state. Retorna el caracter de la celda
 * en donde la snake se va a mover (en el siguiente paso).
 *
 * Esta funcion no deberia modificar nada de state.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  // Obtener la cabeza de la serpiente
  snake_t snake = state->snakes[snum];
  unsigned int row = snake.head_row;
  unsigned int col = snake.head_col;

  // Obtener el carácter de la cabeza para determinar dirección
  char head_char = get_board_at(state, row, col);

  // Calcular la siguiente posición
  unsigned int next_row = get_next_row(row, head_char);
  unsigned int next_col = get_next_col(col, head_char);

  // Retornar el contenido de la celda siguiente
  return get_board_at(state, next_row, next_col);
}


/**
 * Tarea 4.3
 *
 * Funcion de ayuda para update_state. Actualiza la cabeza de la snake...
 *
 * ... en el tablero: agregar un caracter donde la snake se va a mover (¿que caracter?)
 *
 * ... en la estructura del snake: actualizar el row y col de la cabeza
 *
 * Nota: esta funcion ignora la comida, paredes, y cuerpos de otras snakes
 * cuando se mueve la cabeza.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  // Obtener snake y su posición actual
  snake_t* snake = &state->snakes[snum];
  unsigned int row = snake->head_row;
  unsigned int col = snake->head_col;

  // Obtener el carácter actual de la cabeza
  char head_char = get_board_at(state, row, col);

  // Convertir la cabeza anterior en parte del cuerpo
  char body_char = head_to_body(head_char);
  set_board_at(state, row, col, body_char);

  // Calcular nueva posición de la cabeza
  unsigned int next_row = get_next_row(row, head_char);
  unsigned int next_col = get_next_col(col, head_char);

  // Colocar la nueva cabeza en el tablero
  set_board_at(state, next_row, next_col, head_char);

  // Actualizar la estructura de la snake
  snake->head_row = next_row;
  snake->head_col = next_col;
}


/**
 * Tarea 4.4
 *
 * Funcion de ayuda para update_state. Actualiza la cola de la snake...
 *
 * ... en el tablero: colocar un caracter blanco (spacio) donde se encuentra
 * la cola actualmente, y cambiar la nueva cola de un caracter de cuerpo (^<v>)
 * a un caracter de cola (wasd)
 *
 * ...en la estructura snake: actualizar el row y col de la cola
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  // Obtener la snake
  snake_t* snake = &state->snakes[snum];
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;

  // Obtener el carácter de la cola actual (wasd)
  char tail_char = get_board_at(state, row, col);

  // Borrar la cola actual del tablero (poner espacio)
  set_board_at(state, row, col, ' ');

  // Calcular la siguiente posición de la cola
  unsigned int next_row = get_next_row(row, tail_char);
  unsigned int next_col = get_next_col(col, tail_char);

  // Obtener el carácter del cuerpo en la nueva cola
  char next_body = get_board_at(state, next_row, next_col);

  // Convertir ese carácter a cola (wasd)
  char new_tail = body_to_tail(next_body);

  // Actualizar el tablero con la nueva cola
  set_board_at(state, next_row, next_col, new_tail);

  // Actualizar la estructura de la snake
  snake->tail_row = next_row;
  snake->tail_col = next_col;
}


/* Tarea 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    // Si la snake está muerta, no hacemos nada
    if (!state->snakes[i].live) continue;

    // Ver el contenido de la celda a la que se moverá la cabeza
    char next = next_square(state, i);

    // Si la siguiente celda es una pared o parte de cualquier snake, muere
    if (next == '#' || is_snake(next)) {
      state->snakes[i].live = false;
      unsigned int row = state->snakes[i].head_row;
      unsigned int col = state->snakes[i].head_col;
      set_board_at(state, row, col, 'x');
    } else {
      // Mueve la cabeza
      update_head(state, i);

      // Si no comió una fruta, mover la cola
      if (next != '*') {
        update_tail(state, i);
      } else {
        // Si comió fruta, agregar una nueva
        add_food(state);
      }
    }
  }
}


/* Tarea 5 */
game_state_t* load_board(char* filename) {
  // Abrimos el archivo
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    return NULL;
  }

  // Reservamos memoria para la estructura del estado
  game_state_t* state = malloc(sizeof(game_state_t));
  if (state == NULL) {
    fclose(f);
    return NULL;
  }

  // Inicializamos valores
  state->num_snakes = 0;
  state->snakes = NULL;

  // Usamos un array dinámico para filas del tablero
  char** board = NULL;
  unsigned int num_rows = 0;

  // Leemos línea por línea, de forma manual (sin getline)
  char buffer[1024]; // Tamaño máximo temporal por línea
  while (fgets(buffer, sizeof(buffer), f)) {
    size_t len = strlen(buffer);

    // Aseguramos que cada línea termine con '\n'
    if (len > 0 && buffer[len - 1] != '\n') {
      buffer[len] = '\n';
      buffer[len + 1] = '\0';
      len += 1;
    }

    // Reservamos exactamente la memoria necesaria (+1 para '\0')
    char* line = malloc(len + 1);
    if (line == NULL) {
      fclose(f);
      return NULL;
    }
    strcpy(line, buffer);

    // Reasignamos memoria para agregar la línea al tablero
    board = realloc(board, sizeof(char*) * (num_rows + 1));
    board[num_rows] = line;
    num_rows++;
  }

  fclose(f);

  // Guardamos el tablero en el estado
  state->board = board;
  state->num_rows = num_rows;

  return state;
}


/**
 * Tarea 6.1
 *
 * Funcion de ayuda para initialize_snakes.
 * Dada una structura de snake con los datos de cola row y col ya colocados,
 * atravezar el tablero para encontrar el row y col de la cabeza de la snake,
 * y colocar esta informacion en la estructura de la snake correspondiente
 * dada por la variable (snum)
*/
static void find_head(game_state_t* state, unsigned int snum) {
  snake_t* snake = &state->snakes[snum];

  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;
  char current = get_board_at(state, row, col);

  // Recorrer el cuerpo hasta encontrar la cabeza
  while (!is_head(current)) {
    unsigned int next_row = get_next_row(row, current);
    unsigned int next_col = get_next_col(col, current);
    row = next_row;
    col = next_col;
    current = get_board_at(state, row, col);
  }

  // Guardar la posición de la cabeza
  snake->head_row = row;
  snake->head_col = col;
}


/* Tarea 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  // Contar cuántas colas hay (caracteres 'w', 'a', 's', 'd')
  unsigned int count = 0;
  for (unsigned int row = 0; row < state->num_rows; row++) {
    char* line = state->board[row];
    for (unsigned int col = 0; line[col] != '\0'; col++) {
      if (is_tail(line[col])) {
        count++;
      }
    }
  }

  // Asignar memoria para las serpientes
  state->num_snakes = count;
  state->snakes = malloc(sizeof(snake_t) * count);

  // Volver a recorrer para registrar posición de cada cola
  unsigned int index = 0;
  for (unsigned int row = 0; row < state->num_rows; row++) {
    char* line = state->board[row];
    for (unsigned int col = 0; line[col] != '\0'; col++) {
      if (is_tail(line[col])) {
        // Inicializar snake
        state->snakes[index].tail_row = row;
        state->snakes[index].tail_col = col;
        state->snakes[index].live = true;

        // Usar find_head para completar head_row y head_col
        find_head(state, index);

        index++;
      }
    }
  }

  return state;
}
