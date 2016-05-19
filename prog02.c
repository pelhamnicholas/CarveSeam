#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int row;
  int col;
} tuple;

static inline double min2(double a, double b) {
  return (a < b) ? a : b;
}

static inline double min3(double a, double b, double c) {
  double m;

  m = (a < b) ? a : b;
  return (m < c) ? m : c;
}

int main(int arc, char *argv[]) {
  FILE * input_file;
  char line_buf[100000];
  const char delim[] = ", ";
  char * token = NULL;
  int cols, rows, i, j;
  double ** input_table;
  double ** seam_table;
  tuple * seam;
  double temp_min;
  char * output_filename;
  FILE * output_file;

  input_file = fopen(argv[1], "r");

  if (!input_file) {
    printf("INVALID FILENAME!");
    return 1;
  }

  /* Get table dimensions */
  cols = 0;
  fgets(line_buf, 100000, input_file);
  token = strtok(line_buf, delim);
  while (token != NULL) {
    token = strtok(NULL, delim);
    cols++;
  }

  rows = 1;
  while (fgets(line_buf, 100000, input_file) != NULL) {
    rows++;
  }

  /* Create input data table */
  input_table = (double **) malloc(sizeof(double*) * rows);
  for (i = 0; i < rows; i++)
    input_table[i] = (double *) malloc(sizeof(double) * cols);

  rewind(input_file);
  for (i = 0; i < rows; i++) {
    fgets(line_buf, 100000, input_file);
    token = strtok(line_buf, delim);
    input_table[i][0] = atof(token);
    for (j = 1; j < cols; j++) {
      token = strtok(NULL, delim);
      input_table[i][j] = atof(token);
    }
  }

  /* Print input table /
  printf("\nINPUT TABLE\n");
  for (i = 0; i < rows; i++) {
    printf("%f", input_table[i][0]);
    for (j = 1; j < cols; j++) {
      printf(", %f", input_table[i][j]);
    }
    printf("\n");
  }
  */

  /* Create seam data table */
  seam_table = (double **) malloc(sizeof(double*) * rows);
  for (i = 0; i < rows; i++)
    seam_table[i] = (double *) malloc(sizeof(double) * cols);

  /* first row is just a copy */
  for (j = 0; j < cols; j++) {
    seam_table[0][j] = input_table[0][j];
  }
  /* fill in the rest of the table */
  for (i = 1; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      if (j == 0) {
        seam_table[i][j] = input_table[i][j] + 
          min2(seam_table[i-1][j], seam_table[i-1][j+1]);
      } else if (j == rows-1) {
        seam_table[i][j] = input_table[i][j] + 
          min2(seam_table[i-1][j-1], seam_table[i-1][j]);
      } else {
        seam_table[i][j] = input_table[i][j] + 
          min3(seam_table[i-1][j-1], seam_table[i-1][j], seam_table[i-1][j+1]);
      }
    }
  }

  /* Print seam table /
  printf("\nSEAM TABLE\n");
  for (i = 0; i < rows; i++) {
    printf("%f", seam_table[i][0]);
    for (j = 1; j < cols; j++) {
      printf(", %f", seam_table[i][j]);
    }
    printf("\n");
  }
  */

  /* Find minimum seam cost in last row of seam_table */
  seam = (tuple *) malloc(sizeof(tuple) * rows);
  seam[0].row = rows - 1;
  seam[0].col = 0;
  temp_min = seam_table[rows-1][0];
  for (j = 1; j < cols; j++) {
    if (seam_table[rows-1][j] < temp_min) {
      temp_min = seam_table[rows-1][j];
      seam[0].col = j;
    }
  }
  /* Traceback the minimum seam */
  for (i = 1; i < rows; i++) {
    seam[i].row = rows - (i+1);
    if (seam[i-1].col == 0) {
      if (seam_table[rows-(i+1)][seam[i-1].col] < 
          seam_table[rows-(i+1)][seam[i-1].col + 1]) {
        seam[i].col = seam[i-1].col;
      } else {
        seam[i].col = seam[i-1].col + 1;
      }
    } else if (seam[i-1].col == rows-1) {
      if (seam_table[rows-(i+1)][seam[i-1].col - 1] < 
          seam_table[rows-(i+1)][seam[i-1].col]) {
        seam[i].col = seam[i-1].col - 1;
      } else {
        seam[i].col = seam[i-1].col;
      }
    } else {
      if (seam_table[rows-(i+1)][seam[i-1].col - 1] < 
          seam_table[rows-(i+1)][seam[i-1].col]) {
        if (seam_table[rows-(i+1)][seam[i-1].col - 1] < 
            seam_table[rows-(i+1)][seam[i-1].col + 1]) {
          seam[i].col = seam[i-1].col - 1;
        } else {
          seam[i].col = seam[i-1].col + 1;
        }
      } else {
        if (seam_table[rows-(i+1)][seam[i-1].col] < 
            seam_table[rows-(i+1)][seam[i-1].col + 1]) {
          seam[i].col = seam[i-1].col;
        } else {
          seam[i].col = seam[i-1].col + 1;
        }
      }
    }
  }

  /* Print the minum seam */
  output_filename = (char *) malloc(sizeof(argv[1]) + 6*sizeof(char));
  strncpy(output_filename, argv[1], (strlen(argv[1]) - 4));
  strcat(output_filename, "_trace.txt");
  output_file = fopen(output_filename, "w");
  fprintf(output_file, "Min Seam: %f\n", seam_table[seam[0].row][seam[0].col]);
  for (i = 0; i < rows; i++) {
    fprintf(output_file, "[%d, %d, %f]\n", seam[i].row, seam[i].col, 
        input_table[seam[i].row][seam[i].col]);
  }

  /* Deallocate memory */
  free(output_filename);
  free(input_table);
  free(seam_table);

  return 0;
}
