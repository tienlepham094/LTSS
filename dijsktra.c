#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define INFINITY 999

int Read_n(int my_rank, MPI_Comm comm);
MPI_Datatype Build_blk_col_type(int n, int loc_n);
void Read_matrix(int loc_mat[], int n, int loc_n, MPI_Datatype blk_col_mpi_t,
                 int my_rank, MPI_Comm comm);
void Dijkstra_Init(int loc_mat[], int loc_pred[], int loc_dist[], int loc_known[],
                   int my_rank, int loc_n);
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int n,
              MPI_Comm comm);
int Find_min_dist(int loc_dist[], int loc_known[], int loc_n);
void Print_matrix(int global_mat[], int rows, int cols);
void Print_dists(int global_dist[], int n);
void Print_paths(int global_pred[], int n, int glob_dist[]);
double start, end, comm_time, total_time=0;

int main(int argc, char **argv)
{
    int *loc_mat, *loc_dist, *loc_pred, *global_dist = NULL, *global_pred = NULL;
    int my_rank, p, loc_n, n;
    MPI_Comm comm;
    MPI_Datatype blk_col_mpi_t;


    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &p);
    if(my_rank ==0 ){
        total_time = MPI_Wtime();
    }
    n = Read_n(my_rank, comm);
    // printf("My rank: %d\n", my_rank);
    // printf("N: %d\n", n);
    // printf("P: %d\n", p);
    loc_n = n / p;

    loc_mat = malloc(n * loc_n * sizeof(int));
    loc_dist = malloc(loc_n * sizeof(int));
    loc_pred = malloc(loc_n * sizeof(int));
    if (loc_mat == NULL || loc_dist == NULL || loc_pred == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        MPI_Finalize();
        exit(-1);
    }

    blk_col_mpi_t = Build_blk_col_type(n, loc_n);

    if (my_rank == 0)
    {
        global_dist = malloc(n * sizeof(int));
        global_pred = malloc(n * sizeof(int));
    }
    Read_matrix(loc_mat, n, loc_n, blk_col_mpi_t, my_rank, comm);

    // Bat dau do thoi gian
    // start = MPI_Wtime();
    Dijkstra(loc_mat, loc_dist, loc_pred, loc_n, n, comm);
    // end = MPI_Wtime();
    // ket thuc

    // total_time = end - start;

    /* Gather the results from Dijkstra */
    // do thoi gian truyen thong
    comm_time = 0;
    if(my_rank == 0){
        start = MPI_Wtime();
    }
    MPI_Gather(loc_dist, loc_n, MPI_INT, global_dist, loc_n, MPI_INT, 0, comm);
    MPI_Gather(loc_pred, loc_n, MPI_INT, global_pred, loc_n, MPI_INT, 0, comm);
    if(my_rank ==0){
        end = MPI_Wtime();
        comm_time += end - start;
    }
    // ket thuc

    /* Print results */
    if (my_rank == 0)
    {
        total_time += MPI_Wtime() - total_time;
        Print_dists(global_dist, n);
        Print_paths(global_pred, n, global_dist);
        FILE *dijkstra_graph_nT = NULL;
        dijkstra_graph_nT = fopen("dijkstra_graph_nT.txt", "a");
        if (dijkstra_graph_nT == NULL)
        {
            fprintf(stderr, "Error opening output file\n");
            MPI_Finalize();
            exit(-1);
        }
        else
        {
            printf("opening output file\n");
        }
        FILE *dijkstra_graph_nCPUT = NULL;
        dijkstra_graph_nCPUT = fopen("dijkstra_graph_nCPUT.txt", "a");
        if (dijkstra_graph_nCPUT == NULL)
        {
            fprintf(stderr, "Error opening output file\n");
            MPI_Finalize();
            exit(-1);
        }
        else
        {
            printf("opening output file\n");
        }

        fprintf(dijkstra_graph_nT, "%d, ", n);                      // so luong mau
        fprintf(dijkstra_graph_nT, "%f, ", total_time);             // t_w_comm
        fprintf(dijkstra_graph_nT, "%f\n", total_time - comm_time); // t_wo_comm:
        fclose(dijkstra_graph_nT);

        fprintf(dijkstra_graph_nCPUT, "%d, ", p);                      // so luong cpu
        fprintf(dijkstra_graph_nCPUT, "%f, ", total_time);             // t_w_comm
        fprintf(dijkstra_graph_nCPUT, "%f\n", total_time - comm_time); // t_wo_comm:
        free(global_dist);
        free(global_pred);
    }
    // In ket qua do tg
    if (my_rank == 0)
    {
        // printf("total time: %f s\n", total_time);
        printf("t_w_comm: %f s \n", total_time);
        printf("t_wo_comm: %f s \n", total_time - comm_time);
    }
    free(loc_mat);
    free(loc_pred);
    free(loc_dist);
    MPI_Type_free(&blk_col_mpi_t);
    MPI_Finalize();
    return 0;
}

int Read_n(int my_rank, MPI_Comm comm)
{
    int n;

    if (my_rank == 0)
        scanf("%d", &n);
    if(my_rank == 0){
        start = MPI_Wtime();
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, comm);
    if (my_rank == 0)
    {
        end = MPI_Wtime();
        comm_time += end - start;
    }
    return n;
}

MPI_Datatype Build_blk_col_type(int n, int loc_n)
{
    MPI_Aint lb, extent;
    MPI_Datatype block_mpi_t;
    MPI_Datatype first_bc_mpi_t;
    MPI_Datatype blk_col_mpi_t;

    MPI_Type_contiguous(loc_n, MPI_INT, &block_mpi_t);
    MPI_Type_get_extent(block_mpi_t, &lb, &extent);

    /* MPI_Type_vector(numblocks, elts_per_block, stride, oldtype, *newtype) */
    MPI_Type_vector(n, loc_n, n, MPI_INT, &first_bc_mpi_t);

    /* This call is needed to get the right extent of the new datatype */
    MPI_Type_create_resized(first_bc_mpi_t, lb, extent, &blk_col_mpi_t);

    MPI_Type_commit(&blk_col_mpi_t);

    MPI_Type_free(&block_mpi_t);
    MPI_Type_free(&first_bc_mpi_t);

    return blk_col_mpi_t;
}

void Read_matrix(int loc_mat[], int n, int loc_n,
                 MPI_Datatype blk_col_mpi_t, int my_rank, MPI_Comm comm)
{
    int *mat = NULL;
    int i = 0;
    int j = 0;
    if (my_rank == 0)
    {
        mat = malloc(n * n * sizeof(int));
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                scanf("%d", &mat[i * n + j]);
    }
    // Bat dau do thoi gian truyen thong
    // double start = MPI_Wtime();
    if(my_rank ==0){
        start = MPI_Wtime();
    }
    MPI_Scatter(mat, 1, blk_col_mpi_t, loc_mat, n * loc_n, MPI_INT, 0, comm);
    // double end = MPI_Wtime();
    if(my_rank ==0){
        end = MPI_Wtime();
        comm_time += end - start;
    }
    if (my_rank == 0)
    {
        printf("----Matrix----\n");
        Print_matrix(mat, n, n);
    }
    // ket thuc do thoi gian truyen thong

    // if (my_rank == 0) {
    //     printf("Twcomm MPI_Scatter: %f s \n", end - start);
    //     free(mat);
    // }
}

void Dijkstra_Init(int loc_mat[], int loc_pred[], int loc_dist[], int loc_known[],
                   int my_rank, int loc_n)
{
    int loc_v;

    if (my_rank == 0)
        loc_known[0] = 1;
    else
        loc_known[0] = 0;

    for (loc_v = 1; loc_v < loc_n; loc_v++)
        loc_known[loc_v] = 0;

    for (loc_v = 0; loc_v < loc_n; loc_v++)
    {
        loc_dist[loc_v] = loc_mat[0 * loc_n + loc_v];
        loc_pred[loc_v] = 0;
    }
}

void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int n,
              MPI_Comm comm)
{

    int i, loc_v, loc_u, glbl_u, new_dist, my_rank, dist_glbl_u;
    int *loc_known;
    int my_min[2];
    int glbl_min[2];
    double start, end;

    MPI_Comm_rank(comm, &my_rank);
    loc_known = malloc(loc_n * sizeof(int));

    Dijkstra_Init(loc_mat, loc_pred, loc_dist, loc_known, my_rank, loc_n);

    /* Run loop n - 1 times since we already know the shortest path to global
       vertex 0 from global vertex 0 */
    for (i = 0; i < n - 1; i++)
    {
        loc_u = Find_min_dist(loc_dist, loc_known, loc_n);

        if (loc_u != -1)
        {
            my_min[0] = loc_dist[loc_u];
            my_min[1] = loc_u + my_rank * loc_n;
        }
        else
        {
            my_min[0] = INFINITY;
            my_min[1] = -1;
        }

        // do thoi gian truyen thong
        if(my_rank ==0){
            start = MPI_Wtime();
        }
        /* Get the minimum distance found by the processes and store that
           distance and the global vertex in glbl_min
        */
        MPI_Allreduce(my_min, glbl_min, 1, MPI_2INT, MPI_MINLOC, comm);
        if(my_rank ==0){
            end = MPI_Wtime();
            comm_time += start - end;
        }
        // ket thuc do thoi gian truyen thong

        // if (my_rank == 0) {
        //     printf("twcomm MPI_Allreduce %f s \n", i, end - start);
        // }

        dist_glbl_u = glbl_min[0];
        glbl_u = glbl_min[1];

        /* This test is to assure that loc_known is not accessed with -1 */
        if (glbl_u == -1)
            break;

        /* Check if global u belongs to process, and if so update loc_known */
        if ((glbl_u / loc_n) == my_rank)
        {
            loc_u = glbl_u % loc_n;
            loc_known[loc_u] = 1;
        }

        /* For each local vertex (global vertex = loc_v + my_rank * loc_n)
           Update the distances from source vertex (0) to loc_v. If vertex
           is unmarked check if the distance from source to the global u + the
           distance from global u to local v is smaller than the distance
           from the source to local v
         */
        for (loc_v = 0; loc_v < loc_n; loc_v++)
        {
            if (!loc_known[loc_v])
            {
                new_dist = dist_glbl_u + loc_mat[glbl_u * loc_n + loc_v];
                if (new_dist < loc_dist[loc_v])
                {
                    loc_dist[loc_v] = new_dist;
                    loc_pred[loc_v] = glbl_u;
                }
            }
        }
    }
    free(loc_known);
}

int Find_min_dist(int loc_dist[], int loc_known[], int loc_n)
{
    int loc_u = -1, loc_v;
    int shortest_dist = INFINITY;

    for (loc_v = 0; loc_v < loc_n; loc_v++)
    {
        if (!loc_known[loc_v])
        {
            if (loc_dist[loc_v] < shortest_dist)
            {
                shortest_dist = loc_dist[loc_v];
                loc_u = loc_v;
            }
        }
    }
    return loc_u;
}

void Print_matrix(int mat[], int rows, int cols)
{
    int i, j;

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
            if (mat[i * cols + j] == INFINITY)
                printf("inf ");
            else
                printf("%d ", mat[i * cols + j]);
        printf("\n");
    }

    printf("\n");
}

void Print_dists(int global_dist[], int n)
{
    int v;

    printf("  v    dist 0->v\n");
    printf("----   ---------\n");

    for (v = 1; v < n; v++)
    {
        if (global_dist[v] == INFINITY)
        {
            printf("%3d       %5s\n", v, "inf");
        }
        else
            printf("%3d       %4d\n", v, global_dist[v]);
    }
    printf("\n");
}

void Print_paths(int global_pred[], int n, int global_dist[])
{
    int v, w, *path, count, i;

    path = malloc(n * sizeof(int));

    printf("  v     Path 0->v\n");
    printf("----    ---------\n");
    for (v = 1; v < n; v++)
    {
        printf("%3d:    ", v);
        if (global_dist[v] == INFINITY)
        {
            printf("can't reach");
        }
        else
        {
            count = 0;
            w = v;
            while (w != 0)
            {
                path[count] = w;
                count++;
                w = global_pred[w];
            }
            printf("0 ");
            for (i = count - 1; i >= 0; i--)
            {
                printf("%d ", path[i]);
            }
        }
        printf("\n");
    }

    free(path);
}