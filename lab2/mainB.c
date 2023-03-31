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
int link_costs[COST_LENGTH][COST_LENGTH] = {0};
int dv[2][COST_LENGTH][COST_LENGTH];
int broadcast[2][COST_LENGTH] = {0};
int path[COST_LENGTH][COST_LENGTH];
int traffics[COST_LENGTH * COST_LENGTH][3] = {0};
// int route[COST_LENGTH][COST_LENGTH];

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
// struct position
// {
//   int now;
//   int distance;
//   int seq[COST_LENGTH];
// };
// void bfs(int dv_local[][COST_LENGTH], int src, int dst, int dis)
// {
//   int i, j, front = 0, tail = 1;
//   struct position positions[COST_LENGTH * COST_LENGTH * COST_LENGTH];
//   positions[0].now = src;
//   positions[0].distance = 0;
//   positions[0].seq[0] = 1;
//   positions[0].seq[1] = src;
//   int times = 10;
//   while (front < tail)
//   {
//     struct position *head, *next;
//     head = positions + front;
//     front++;
//     int v = head->seq[0];
//     printf("now=%d, dis=%d ", head->now, head->distance);
//     printf("front=%d, tail=%d\n", front, tail);
//     for (i = 0; i < num_nodes; i++)
//     {
//       // printf("tip: %d=?=%d, %d, %d, %d\n", i, head->now, link_costs[head->now][i], dv_local[src][i], head->distance);
//       if (i == head->now || link_costs[head->now][i] == -1 || dv_local[src][i] == -1 || (head->distance + link_costs[head->now][i] +dv_local[i][dst]!= dv_local[src][dst]))
//         continue;
//       printf("(%d->%d)\n", head->now, i);
//       if (i == dst)
//       {
//         // printf("print\n");
//         for (j = 1; j <= v; j++)
//           printf("%d>", head->seq[j]);
//         printf("%d\n", dst);
//         return;
//       }
//       next = &positions[tail++];

//       next = positions + tail;
//       tail++;
//       next->now = i;
//       next->distance = dv_local[src][i];
//       for (j = 1; j <= v; j++)
//         next->seq[j] = head->seq[j];
//       next->seq[0] = v + 1;
//       next->seq[v + 1] = i;
//       if (times == 0)
//       exit(0);
//       // else
//       times--;
//       // printf("%d-%d;", front, tail);
//       // printf("times=%d\n",times);
//       // printf("\nnext: %d %d\n",next->now,next->distance);
//     }
//     // exit(0);
//   }
//   // exit(0);
// }
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
      link_costs[i][j] = numbers[l];
      dv[k % 2][i][j] = numbers[l];
      if (i == j)
        path[i][j] = 0;
      else if (numbers[l] == -1)
        path[i][j] = -1;
      else
        path[i][j] = j;
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
  int p, flag = 1;
  for (k = 1; k <= K; k++)
  {
    // TODO: if some nodes broadcast the dv, update dv
    if (flag)
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
              else if (dv[k % 2][i][l] == -1 || (dv[k % 2][i][l] != -1 && dv[k % 2][i][l] > (dv[(k - 1) % 2][j][l] + link_costs[i][j])))
              {
                dv[k % 2][i][l] = dv[(k - 1) % 2][j][l] + link_costs[i][j];
                path[i][l] = j;
              }
              else if (dv[k % 2][i][l] != -1 && dv[k % 2][i][l] == (dv[(k - 1) % 2][j][l] + link_costs[i][j]))
              {
                if (l == j || path[i][l] == -1)
                  path[i][l] = j;
                else
                  path[i][l] = MIN(j, path[i][l]);
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
    // TODO: detect whether converging, and make the flag
    flag = 0;
    for (i = 0; i < num_nodes; i++)
      flag += broadcast[k % 2][i];
    // print_kout(k, k);
    printf("k=%d:\n", k);
    // printf("%d\n",V);
    // exit(0);
    for (i = 0; i < V; i++)
    {
      printf("%d %d %d ", traffics[i][0], traffics[i][1], traffics[i][2]);
      if (dv[k % 2][traffics[i][0]][traffics[i][1]] == -1)
        printf("null\n");
      else
      {
        printf("%d", traffics[i][0]);
        int t = traffics[i][0];
        while (1)
        {
          t = path[t][traffics[i][1]];
          printf(">%d", t);
          if (t == traffics[i][1])
          {
            printf("\n");
            break;
          }
        }
      }
      // continue;
      // bfs(dv[k % 2], traffics[i][0], traffics[i][1], dv[k % 2][traffics[i][0]][traffics[i][1]]);
    }
  }
}
