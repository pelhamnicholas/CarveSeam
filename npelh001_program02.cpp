#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace cv;

typedef struct {
  int row;
  int col;
} tuple;

static inline long min2_i(long a, long b) {
  return (a < b) ? a : b;
}

static inline long min3_i(long a, long b, long c) {
  long m;

  m = (a < b) ? a : b;
  return (m < c) ? m : c;
}

static inline double min2(double a, double b) {
  return (a < b) ? a : b;
}

static inline double min3(double a, double b, double c) {
  double m;

  m = (a < b) ? a : b;
  return (m < c) ? m : c;
}

static inline tuple get_pixel(int row, int col) {
  tuple pixel;
  pixel.row = row;
  pixel.col = col;
  return pixel;
}

static inline int distance(Mat &I, tuple rgb1, tuple rgb2) {
  int tmp, sum = 0;
  Vec3b pixel_1 = I.at<Vec3b>(rgb1.row, rgb1.col);
  Vec3b pixel_2 = I.at<Vec3b>(rgb2.row, rgb2.col);

  for (int i = 0; i < 3; i++) {
    tmp = ((int)pixel_1[i] - (int)pixel_2[i]);
    sum += tmp * tmp;
  }
  return sum;
}

int energy(Mat &I, int row, int col, int height, int width) {
  int result = 0;
  tuple pixel = get_pixel(row, col);

  for (int c = col-1; c < col+2; c++) {
    if (0 <= c && c < width) {
      result += distance(I, pixel, get_pixel(row, c));
    }
  }
  for (int r = row-1; r < row+2; r++) {
    if (0 <= r && r < height) {
      result += distance(I, pixel, get_pixel(r, col));
    }
  }
  if (0 <= col && col < width) {
    result *= 2;
  }
  return result;
}

int carve_image(int argc, char *argv[]);
int carve_text(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  char * input_file;
  if (argc < 2) {
    printf("  Missing file name parameter!\n");
    return 1;
  }

  input_file = argv[1];

  if (strcmp(&input_file[(strlen(input_file) > 4) 
        ? (strlen(input_file)-4) : 0], ".txt") == 0) {
    printf("%s\n", argv[1]);
    carve_text(argc, argv);
    return 0;
  } else if ((strcmp(&input_file[(strlen(input_file) > 4) 
        ? (strlen(input_file)-4) : 0], ".jpg") == 0)
        || (strcmp(&input_file[(strlen(input_file) > 4) 
        ? (strlen(input_file)-4) : 0], ".png") == 0)) {
    carve_image(argc, argv);
    return 0;
  } else {
    // incorrect file type
    printf("  Incorrect file extension!\n");
    printf("  Input file must be .txt, .jpg, or .png\n");
    return 1;
  }

  //return 0;
}

int carve_image(int argc, char *argv[]) {
  Mat I;

  if (argc < 2) {
    cout << "  MISSING FILENAME PARAMETER!" << endl;
    return 1;
  }

  I = imread(argv[1], CV_LOAD_IMAGE_COLOR);

  int rows = I.rows;
  int cols = I.cols;

  cout << "Rows: " << rows << " Cols: " << cols << endl;

  /* Create energy data table from energy cost function */
  int ** energy_table = (int **) malloc(sizeof(int *) * rows);
  for (int i = 0; i < rows; i++)
    energy_table[i] = (int *) malloc(sizeof(int) * cols);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      energy_table[i][j] = energy(I, i, j, rows, cols);
    }
  }

  /* Print energy table
  //char * output_text = (char *) malloc(sizeof(argv[1]) + 6*sizeof(char));
  //strncpy(output_text, argv[1], (strlen(argv[1]) - 4));
  //strcat(output_text, "_trace.txt");
  FILE * output_file = fopen("girl_energy.txt", "w");

  fprintf(output_file, "\nENERGY TABLE\n");
  for (int i = 0; i < rows; i++) {
    fprintf(output_file, "%7d", energy_table[i][0]);
    for (int j = 1; j < cols; j++) {
      fprintf(output_file, ", %7d", energy_table[i][j]);
    }
    fprintf(output_file, "\n");
  }
  fclose(output_file);
  */

  /* Create seam data table */
  long ** seam_table = (long **) malloc(sizeof(long *) * rows);
  for (int i = 0; i < rows; i++)
    seam_table[i] = (long *) malloc(sizeof(long) * cols);

  /* first row is just a copy */
  for (int j = 0; j < cols; j++) {
    seam_table[0][j] = (long) energy_table[0][j];
  }
  /* fill in the rest of the table */
  for (int i = 1; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (j == 0) {
        seam_table[i][j] = (long)energy_table[i][j] + 
          min2_i(seam_table[i-1][j], seam_table[i-1][j+1]);
      } else if (j == cols-1) {
        seam_table[i][j] = (long)energy_table[i][j] + 
          min2_i(seam_table[i-1][j-1], seam_table[i-1][j]);
      } else {
        seam_table[i][j] = (long)energy_table[i][j] + 
          min3_i(seam_table[i-1][j-1], seam_table[i-1][j], seam_table[i-1][j+1]);
      }
    }
  }

  /* Print seam table
  output_file = fopen("girl_seam.txt", "w");
  fprintf(output_file, "\nSEAM TABLE\n");
  for (int i = 0; i < rows; i++) {
    fprintf(output_file, "%7ld", seam_table[i][0]);
    for (int j = 1; j < cols; j++) {
      fprintf(output_file, ", %7ld", seam_table[i][j]);
    }
    fprintf(output_file, "\n");
  }
  fclose(output_file);
  */

  /* Find minimum seam cost in last row of seam_table */
  tuple * seam = (tuple *) malloc(sizeof(tuple) * rows);
  seam[0].row = rows - 1;
  seam[0].col = 0;
  int temp_min = seam_table[rows-1][0];
  for (int j = 1; j < cols; j++) {
    if (seam_table[rows-1][j] < temp_min) {
      temp_min = seam_table[rows-1][j];
      seam[0].col = j;
    }
  }
  /* Traceback the minimum seam */
  for (int i = 1; i < rows; i++) {
    seam[i].row = rows - (i+1);
    if (seam[i-1].col == 0) {
      if (seam_table[rows-(i+1)][seam[i-1].col] < 
          seam_table[rows-(i+1)][seam[i-1].col + 1]) {
        seam[i].col = seam[i-1].col;
      } else {
        seam[i].col = seam[i-1].col + 1;
      }
    } else if (seam[i-1].col == cols-1) {
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
  printf("Min Seam: %ld\n", seam_table[seam[0].row][seam[0].col]);

  Vec3b color(0,0,255);
  for (int i = 0; i < rows; i++) {
    I.at<Vec3b>(seam[i].row, seam[i].col) = color;
  }
  namedWindow(argv[1], CV_WINDOW_AUTOSIZE);
  imshow(argv[1], I);
  waitKey();

  /* Print the minum seam in a text file
  char * output_text = (char *) malloc(sizeof(argv[1]) + 6*sizeof(char));
  strncpy(output_text, argv[1], (strlen(argv[1]) - 4));
  strcat(output_text, "_trace.txt");
  FILE * output_file = fopen(output_text, "w");
  output_file = fopen("girl_trace.txt", "w");
  fprintf(output_file, "Min Seam: %ld\n", seam_table[seam[0].row][seam[0].col]);
  for (int i = 0; i < rows; i++) {
    fprintf(output_file, "[%d, %d, %d]\n", seam[i].row, seam[i].col, 
        energy_table[seam[i].row][seam[i].col]);
  }
  fclose(output_file);
  */

  /* Save the image */
  char * output_filename = (char *) malloc(sizeof(argv[1]) + 6*sizeof(char));
  strncpy(output_filename, argv[1], (strlen(argv[1]) - 4));
  strcat(output_filename, "_trace.jpg");
  imwrite(output_filename, I);

  /* Deallocate memory */
  free(output_filename);
  free(seam_table);
  free(seam);

  return 0;
}

int carve_text(int argc, char *argv[]) {
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

  if (argc < 2) {
    printf("Missing filename parameter\n");
    return 1;
  }

  input_file = fopen(argv[1], "r");

  if (!input_file) {
    printf("INVALID FILENAME!\n");
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
      } else if (j == cols-1) {
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
    } else if (seam[i-1].col == cols-1) {
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
  free(seam);

  return 0;
}
