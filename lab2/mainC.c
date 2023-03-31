#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define COST_LENGTH 20
#define MIN(a, b) (a) > (b) ? (b) : (a)
int num_nodes;
int K;
int V;
char *filename_topo, *filename_traffic;
int end_flag_for_read;
int link_costs[2][COST_LENGTH][COST_LENGTH] = {0};
int dv[2][COST_LENGTH][COST_LENGTH];
int broadcast[2][COST_LENGTH] = {0};
int path[COST_LENGTH][COST_LENGTH];
int hop[2][COST_LENGTH][COST_LENGTH] = {0};
int count[2][COST_LENGTH][COST_LENGTH] = {0};

int traffics[COST_LENGTH * COST_LENGTH][3] = {0};
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
void print_k(int k, int k0)
{
  // printf("k=%d:\n", k0);
  int i, j;
  printf("distance vector:\n");
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
  printf("link cost:\n");
  for (i = 0; i < num_nodes; i++)
  {
    printf("node-%d: ", i);
    for (j = 0; j < num_nodes; j++)
    {
      printf("%d", link_costs[k % 2][i][j]);
      if (j != num_nodes - 1)
        printf(" ");
    }
    printf("\n");
  }
}
void print_kout(int k, int k0)
{
  // printf("k=%d:\n", k0);
  int i, j;
  printf("broadcast:\n");
  for (i = 0; i < num_nodes; i++)
  {
    printf("%d ", broadcast[k % 2][i]);
  }
  printf("\n");
  printf("path:\n");
  for (i = 0; i < num_nodes; i++)
  {
    printf("node-%d: ", i);
    for (j = 0; j < num_nodes; j++)
    {
      printf("%d", path[i][j]);
      if (j != num_nodes - 1)
        printf(" ");
    }
    printf("\n");
  }
  printf("hop:\n");
  for (i = 0; i < num_nodes; i++)
  {
    printf("node-%d: ", i);
    for (j = 0; j < num_nodes; j++)
    {
      printf("%d", hop[k % 2][i][j]);
      if (j != num_nodes - 1)
        printf(" ");
    }
    printf("\n");
  }
  printf("count:\n");
  for (i = 0; i < num_nodes; i++)
  {
    printf("node-%d: ", i);
    for (j = 0; j < num_nodes; j++)
    {
      printf("%d", count[k % 2][i][j]);
      if (j != num_nodes - 1)
        printf(" ");
    }
    printf("\n");
  }
}
void main(int argc, char *argv[])
{
  int i, j, k = 0, l;
  K = atoi(argv[1]);

  // TODO: read topo
  filename_topo = (char *)malloc(FILENAME_MAX * sizeof(char));
  strcpy(filename_topo, argv[2]);
  FILE *file_topo = fopen(filename_topo, "r");
  if (file_topo == NULL)
  {
    printf("open file topo error");
    return;
  }
  int numbers[110] = {0};
  read_ints(file_topo, numbers);
  fclose(file_topo);
  num_nodes = sqrt(1.0 * numbers[0]);
  for (i = 0, l = 1; i < num_nodes; i++)
    for (j = 0; j < num_nodes; j++, l++)
    {
      link_costs[k % 2][i][j] = numbers[l];
      dv[k % 2][i][j] = numbers[l];
      if (i == j)
      {
        path[i][j] = 0;
      }
      else if (numbers[l] == -1)
      {
        path[i][j] = -1;
      }
      else
      {
        path[i][j] = j;
      }
      hop[k % 2][i][j] = 0;
      count[k % 2][i][j] = 0;
      broadcast[k % 2][i] = 1;
    }
  // TODO: read traffic
  filename_traffic = (char *)malloc(FILENAME_MAX * sizeof(char));
  strcpy(filename_traffic, argv[3]);
  FILE *file_traffic = fopen(filename_traffic, "r");
  if (file_traffic == NULL)
  {
    printf("open file trafic error");
    return;
  }
  int traffic_numbers[COST_LENGTH * COST_LENGTH * 3 + 5] = {0};
  read_ints(file_traffic, traffic_numbers);
  fclose(file_traffic);
  V = traffic_numbers[0] / 3;
  for (i = 0, k = 0; i < traffic_numbers[0]; i++)
  {
    traffics[k][i % 3] = traffic_numbers[i + 1];
    if (i % 3 == 2)
      k++;
  }
  // TODO: loop k times
  int p, flag = 1;
  for (k = 1; k <= K; k++)
  {
    // TODO: if some nodes broadcast the dv, update dv

    printf("k=%d:\n", k);
    // print_k(k-1, k-1);
    if (flag)
    {
      for (i = 0; i < num_nodes; i++)
      {
        for (j = 0; j < num_nodes; j++)
        {
          dv[k % 2][i][j] = (i != j ? -1 : 0);
          hop[k % 2][i][j] = hop[(k - 1) % 2][i][j];
          count[k % 2][i][j] = count[(k - 1) % 2][i][j];
        }
        for (j = 0; j < num_nodes; j++)
        {
          if (broadcast[j] == 0 || i == j || link_costs[(k - 1) % 2][i][j] == -1)
            continue;
          else
            for (l = 0; l < num_nodes; l++)
            {
              if (i == l || dv[(k - 1) % 2][j][l] == -1)
                continue;
              else if ((dv[k % 2][i][l] == -1) || (dv[k % 2][i][l] != -1 && dv[k % 2][i][l] > (dv[(k - 1) % 2][j][l] + link_costs[(k - 1) % 2][i][j])))
              {
                dv[k % 2][i][l] = dv[(k - 1) % 2][j][l] + link_costs[(k - 1) % 2][i][j];
                path[i][l] = j;
                hop[k % 2][i][l] = 1 + hop[(k - 1) % 2][j][l];
                count[k % 2][i][l] = j + count[(k - 1) % 2][j][l];
              }
              else if (dv[k % 2][i][l] != -1 && dv[k % 2][i][l] == (dv[(k - 1) % 2][j][l] + link_costs[(k - 1) % 2][i][j]))
              {
                if (hop[(k - 1) % 2][j][l] + 1 < hop[k % 2][i][l] || (hop[(k - 1) % 2][j][l] + 1 == hop[k % 2][i][l] && count[(k - 1) % 2][j][l] + j < count[k % 2][i][l]))
                {
                  path[i][l] = j;
                  hop[k % 2][i][l] = 1 + hop[(k - 1) % 2][j][l];
                  count[k % 2][i][l] = j + count[(k - 1) % 2][j][l];
                }
              }
            }
        }
        broadcast[k % 2][i] = 0;
        for (j = 0; j < num_nodes; j++)
          if (dv[k % 2][i][j] != dv[(k - 1) % 2][i][j])
          {
            broadcast[k % 2][i] = 1;
            break;
          }
      }
    }
    // TODO: detect route and change link cost
    for (i = 0; i < num_nodes; i++)
    {
      for (j = 0; j < num_nodes; j++)
      {
        link_costs[k % 2][i][j] = (link_costs[(k - 1) % 2][i][j] == -1) ? -1 : 0;
      }
    }
    for (i = 0; i < V; i++)
    {
      printf("%d %d %d ", traffics[i][0], traffics[i][1], traffics[i][2]);
      if (dv[k % 2][traffics[i][0]][traffics[i][1]] == -1)
        printf("null\n");
      else
      {
        int traverse[COST_LENGTH] = {0};
        int sequence[COST_LENGTH] = {0};
        printf("%d", traffics[i][0]);
        int t = traffics[i][0], traverse_flag = 1;
        traverse[t] = 1;
        sequence[++sequence[0]] = t;
        while (1)
        {
          t = path[t][traffics[i][1]];
          printf(">%d", t);
          if (traverse[t])
          {
            printf("(drop)\n");
            traverse_flag = 0;
            break;
          }
          traverse[t] = 1;
          sequence[++sequence[0]] = t;
          if (t == traffics[i][1])
          {
            printf("\n");
            break;
          }
        }
        if (traverse_flag)
        {
          for (j = 1; j < sequence[0]; j++)
          {
            link_costs[k % 2][sequence[j]][sequence[j + 1]] += traffics[i][2];
            link_costs[k % 2][sequence[j + 1]][sequence[j]] += traffics[i][2];
          }
        }
      }
    }
    for (i = 0; i < num_nodes; i++)
    {
      for (j = 0; j < num_nodes; j++)
      {
        if (link_costs[k % 2][i][j] != link_costs[(k - 1) % 2][i][j])
        {
          broadcast[k % 2][i] = 1;
          for (l = 0; l < num_nodes; l++)
          {
            if (i == l || link_costs[k % 2][i][l] == -1 || path[i][l] != j)
              continue;
            // dv[k % 2][i][l] += link_costs[k % 2][i][j] - link_costs[(k - 1) % 2][i][j];
          }
        }
      }
    }

    // print_kout(k, k);
    // TODO: detect dv broadcasting
    flag = 0;
    for (i = 0; i < num_nodes; i++)
      flag += broadcast[k % 2][i];
  }
}
