#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define COST_LENGTH 20
#define MIN(a, b) (a) > (b) ? (b) : (a)
int num_nodes;
int K;
char *filename;
int end_flag_for_read;
int link_costs[COST_LENGTH][COST_LENGTH] = {0};
int dv[2][COST_LENGTH][COST_LENGTH];
int broadcast[2][COST_LENGTH] = {0};

void read_ints(FILE *file, int nums[])
{
  int k = 1, num = 0, flag = -1;
  char c;
  while (1)
  {
    if ((c = fgetc(file)) == EOF)
    {
      if (flag == 1)
        nums[++nums[0]] = num;
      return;
    }
    if (c == ' ' || c == '\n')
    {
      if (flag == 1)
        nums[++nums[0]] = num;
      k = 1;
      num = 0;
      flag = -1;
      continue;
    }
    flag = 1;
    if (c == '-')
      k = -1;
    else
    {
      num = k * (num * 10 + (int)(c - '0'));
      k = 1;
    }
  }
  return;
}

void print_kout(int k, int k0)
{
  printf("k=%d:\n", k0);
  int i, j;
  for (i = 0; i < num_nodes; i++)
  {
    printf("node-%d: ", i);
    for (j = 0; j < num_nodes; j++)
    {
      printf("%d", dv[k % 2][i][j]);
      if (j != num_nodes - 1)
        printf(" ");
    }
    printf("\n");
  }
}
void main(int argc, char *argv[])
{
  K = atoi(argv[1]);
  filename = (char *)malloc(FILENAME_MAX * sizeof(char));
  strcpy(filename, argv[2]);
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("open file error");
    return;
  }
  int numbers[110] = {0};
  int i, j, k = 0, l;
  read_ints(file, numbers);
  fclose(file);
  num_nodes = sqrt(1.0 * numbers[0]);
  for (i = 0, l = 1; i < num_nodes; i++)
    for (j = 0; j < num_nodes; j++, l++)
    {
      link_costs[i][j] = numbers[l];
      dv[k % 2][i][j] = numbers[l];
      broadcast[k % 2][i] = 1;
    }
  print_kout(k, k);
  int p, flag = 1;
  if (K < 10)
    K = 10;
  for (k = 1; k <= K; k++)
  {
    if (flag)//if some nodes broadcast the dv
    {
      for (i = 0; i < num_nodes; i++)
      {
        for (j = 0; j < num_nodes; j++)
          dv[k % 2][i][j] = (i != j ? -1 : 0);
        for (j = 0; j < num_nodes; j++)
          if (broadcast[j] == 0 || i == j || link_costs[i][j] == -1)
            continue;
          else
            for (l = 0; l < num_nodes; l++)
              if (i == l || dv[(k - 1) % 2][j][l] == -1)
                continue;
              else
                dv[k % 2][i][l] = dv[k % 2][i][l] == -1 ? (dv[(k - 1) % 2][j][l] + link_costs[i][j]) : MIN(dv[k % 2][i][l], (dv[(k - 1) % 2][j][l] + link_costs[i][j]));
        broadcast[k % 2][i] = 0;
        for (j = 0; j < num_nodes; j++)
          if (dv[k % 2][i][j] != dv[(k - 1) % 2][i][j])
          {
            broadcast[k % 2][i] = 1;
            break;
          }
      }
    }
    if (k < 5 || k % 10 == 0)
      print_kout(k, k);
    flag = 0;//detect whether converging
    for (i = 0; i < num_nodes; i++)
      flag += broadcast[k % 2][i];
    if (flag == 0) // converge k=9->k=10 to detect; converge k=10->k=11 to detect
    {
      if (k >= 10)//finishing condition
      {
        if (k % 10 > 1)
          print_kout(k, 10 * (1 + (int)k / 10));
        break;
      }
    }
  }
}
