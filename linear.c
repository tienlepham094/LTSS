#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]);
void timestamp();
void shuffle(int *array, int n);

int main(int argc, char *argv[])
{
    int DEBUG = 0;
    int EVAL_STEP = 100;
    int MAX_STEP = 300;
    int BATCH_SIZE = 1024;
    double LR = 0.001;

    double part_mse = 0;
    double mse = 0;

    int n_samples;
    int data_dim;

    int machine_id;
    int n_machines;

    double totalTime, comTime, comSTime;
    double T_w_com = 0.0; // Total time with communication
    double T_wo_com = 0.0;

    FILE *file;
    file = fopen("linear.train", "r");

    fscanf(file, "%d", &n_samples);
    fscanf(file, "%d", &data_dim);
    // printf("Data dim %d\n", data_dim);
    // printf("Samples %d\n", n_samples);
    // Read matrix data , X = original values, append 1 for bias
    if(n_samples <1000){
        BATCH_SIZE = 64;
    }
    double **X = (double **)malloc(n_samples * sizeof(double *));
    for (int i = 0; i < n_samples; ++i)
        X[i] = malloc(data_dim * sizeof(double));

    double *Y = (double *)malloc(n_samples * sizeof(double));
    int n_batches = (int)n_samples / BATCH_SIZE;
   

    // data_dim = data_dim -1;
    double *W = (double *)malloc(data_dim * sizeof(double));
    double *grad = (double *)malloc(data_dim * sizeof(double));
    double *part_grad = (double *)malloc(data_dim * sizeof(double));

    int *index = (int *)malloc(n_samples * sizeof(int));

    for (int i = 0; i < n_samples; i++)
    {
        for (int j = 0; j < data_dim - 1; j++)
            if (!fscanf(file, "%lf", &X[i][j]))
                break;
        X[i][data_dim-1] = 1; // set bias
        if (!fscanf(file, "%lf", &Y[i]))
            break;
    }

    fclose(file);

    /*
        Initialize MPI.
    */
    MPI_Init(&argc, &argv);

    /*
        Get the number of processes.
    */
    MPI_Comm_size(MPI_COMM_WORLD, &n_machines);

    /*
        Determine this processes's rank.
    */
    MPI_Comm_rank(MPI_COMM_WORLD, &machine_id);

    // Start Total Time
    if (machine_id == 0)
    {
        totalTime = MPI_Wtime();
    }
    comTime = 0;

    int batch_size_per_machine = (int)BATCH_SIZE / n_machines;

    double **X_batch = (double **)malloc(batch_size_per_machine * sizeof(double *));
    for (int i = 0; i < batch_size_per_machine; ++i)
        X_batch[i] = malloc(data_dim * sizeof(double));

    double *Y_batch = (double *)malloc(batch_size_per_machine * sizeof(double));
    double *temp_values = (double *)malloc(batch_size_per_machine * sizeof(double));

    if (machine_id == 0)
    {
        timestamp();
        if (DEBUG)
        {
            printf("\nX data\n");
            for (int i = 0; i < n_samples; i++)
            {
                for (int j = 0; j < data_dim; j++)
                {
                    printf("%lf ", X[i][j]);
                }
                printf("\n");
            }
            printf("\n");

            printf("Y data\n");
            for (int i = 0; i < n_samples; i++)
            {
                printf("%lf ", Y[i]);
            }
            printf("\n\n");

            printf("Number of processes: %d\n", n_machines);
            printf("Number of steps: %d\n", MAX_STEP);
        }

        // Index init
        for (int i = 0; i < n_samples; i++)
            index[i] = i;

        // Weight init
        srand(time(NULL));
        for (int i = 0; i < data_dim; i++)
        {
            W[i] = (double)rand() / (double)(RAND_MAX);
        }

        if (DEBUG)
        {
            printf("Number of batch: %d\n\n", n_batches);
            // Print init weight
            for (int i = 0; i < data_dim; i++)
                printf("Init W %lf\n", W[i]);
            printf("\n");
        }
    }

    // BCast init weight to all machine
    comSTime = MPI_Wtime();
    MPI_Bcast(W, data_dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    comTime += MPI_Wtime() - comSTime;

    int step = 0;
    while (step < MAX_STEP)
    {
        double start_step = MPI_Wtime();
        part_mse = 0;
        if (machine_id == 0)
        {
            shuffle(index, n_samples);
            comSTime = MPI_Wtime();
        }

        // BCast shuffled index to all machine
        MPI_Bcast(index, n_samples, MPI_INT, 0, MPI_COMM_WORLD);
        if (machine_id == 0)
        {
            comTime += MPI_Wtime() - comSTime;
        }

        int batch_id = 0;
        int start = 0;

        while (batch_id < n_batches)
        {
            start = batch_id * BATCH_SIZE;
            for (int i = 0; i < batch_size_per_machine; i++)
            {
                for (int j = 0; j < data_dim; j++)
                    X_batch[i][j] = X[index[start + machine_id * batch_size_per_machine + i]][j];
                Y_batch[i] = Y[index[start + machine_id * batch_size_per_machine + i]];
            }

            for (int i = 0; i < batch_size_per_machine; ++i)
            {
                temp_values[i] = 0;
            }
            // XW-Y
            for (int i = 0; i < batch_size_per_machine; ++i)
            {
                for (int j = 0; j < data_dim; ++j)
                {
                    temp_values[i] += X_batch[i][j] * W[j];
                }

                if (step % EVAL_STEP == 0)
                {

                    part_mse += (temp_values[i] - Y_batch[i]) * (temp_values[i] - Y_batch[i]);
                    // printf("temp value %.4f y bath %.4f\n", temp_values[i], Y_batch[i]);
                    // printf("Step %d %d Part mse: %.4f\n", step, machine_id, part_mse);
                }

                temp_values[i] -= Y_batch[i];
            }
            // X.T(XW-Y)
            for (int i = 0; i < data_dim; ++i)
            {
                part_grad[i] = 0;
                for (int j = 0; j < batch_size_per_machine; ++j)
                {
                    part_grad[i] += X_batch[j][i] * temp_values[j];
                }
            }
            T_wo_com += MPI_Wtime() - start_step;
            /*
                Combine grad and update weight using REDUCE
            */
            comSTime = MPI_Wtime();
            MPI_Reduce(part_grad, grad, data_dim, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            if (machine_id == 0)
            {
                for (int i = 0; i < data_dim; i++)
                {
                    W[i] = W[i] - LR * grad[i];
                }
            }
            // BCast updated weight to all machine
            MPI_Bcast(W, data_dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            if (machine_id == 0)
            {
                comTime += MPI_Wtime() - comSTime;
            }
            T_w_com += MPI_Wtime() - start_step;
            /* ===================================================================================*/

            /*
                Combine grad and update weight using ALLREDUCE
            */
            /* ===================================================================================*/
            // ierr = MPI_Allreduce(part_grad, grad, data_dim, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

            // for (int i =0;i<data_dim;i++) {
            //     W[i] = W[i] -LR * grad[i];
            // }
            /* ===================================================================================*/
            if (DEBUG)
            {
                for (int i = 0; i < data_dim; i++)
                    printf("Step %d Machine %d: W %lf\n", step, machine_id, W[i]);
            }
            batch_id++;
        }
        if (step % EVAL_STEP == 0)
        {
            comSTime = MPI_Wtime();
            MPI_Reduce(&part_mse, &mse, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            if (machine_id == 0)
            {
                comTime += MPI_Wtime() - comSTime;
                if(mse != 0){
                    mse = sqrt(mse / (n_batches * BATCH_SIZE));
                }
                printf("Step %d mse %f\n", step, mse);
            }
        }
        step++;
    }
    if (DEBUG)
    {
        for (int i = 0; i < data_dim; i++)
            printf("Machine %d: W %lf\n", machine_id, W[i]);
    }
    // totalTime = MPI_Wtime() - totalTime;

    /*
        Evaluation in test set
    */

    file = fopen("linear.test", "r");
    int n_samples_test;
    int data_dim_test;

    fscanf(file, "%d", &n_samples_test);
    fscanf(file, "%d", &data_dim_test);

    double **X_test = (double **)malloc(n_samples_test * sizeof(double *));
    for (int i = 0; i < n_samples_test; ++i)
        X_test[i] = malloc(data_dim * sizeof(double));

    double *Y_test = (double *)malloc(n_samples_test * sizeof(double));

    n_batches = (int)n_samples_test / BATCH_SIZE;
  
    if (data_dim_test != data_dim)
    {
        printf("File test error\n");
        exit(1);
    }

    for (int i = 0; i < n_samples_test; i++)
    {
        for (int j = 0; j < data_dim - 1; j++)
            if (!fscanf(file, "%lf", &X_test[i][j]))
                break;
        X_test[i][data_dim - 1] = 1;
        if (!fscanf(file, "%lf", &Y_test[i]))
            break;
    }

    fclose(file);

    int batch_id = 0;
    int start = 0;
    part_mse = 0;
    while (batch_id < n_batches)
    {
        start = batch_id * BATCH_SIZE;
        for (int i = 0; i < batch_size_per_machine; i++)
        {
            for (int j = 0; j < data_dim; j++)
                X_batch[i][j] = X_test[start + machine_id * batch_size_per_machine + i][j];
            Y_batch[i] = Y_test[start + machine_id * batch_size_per_machine + i];
        }

        for (int i = 0; i < batch_size_per_machine; ++i)
        {
            temp_values[i] = 0;
        }
        // XW-Y
        for (int i = 0; i < batch_size_per_machine; ++i)
        {
            for (int j = 0; j < data_dim; ++j)
            {
                temp_values[i] += X_batch[i][j] * W[j];
            }
            part_mse += (temp_values[i] - Y_batch[i]) * (temp_values[i] - Y_batch[i]);
        }
        batch_id++;
    }
    comSTime = MPI_Wtime();
    MPI_Reduce(&part_mse, &mse, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (machine_id == 0)
    {
        comTime += MPI_Wtime() - comSTime;
        if(mse !=0){
            mse = sqrt(mse / (n_batches * BATCH_SIZE));
        }
        printf("Test mse %lf\n", mse);
    }

    /*
        Terminate MPI.
    */
    MPI_Finalize();

    /*
        Terminate.
    */
    if (machine_id == 0)
    {
        printf("W data\n");
        for (int i = 0; i < data_dim; i++)
        {
            printf("%lf ", W[i]);
        }
        printf("\n");

        printf("\n");
        printf("Master process:\n");
        printf("  Normal end of execution.\n");
        printf("\n");
        timestamp();
    }

    /*
        Free all data
    */
    for (int i = 0; i < n_samples; ++i)
        free(X[i]);
    free(X);
    free(Y);
    for (int i = 0; i < n_samples_test; ++i)
        free(X_test[i]);
    free(X_test);
    free(Y_test);
    free(W);
    free(grad);
    free(part_grad);
    free(index);
    for (int i = 0; i < batch_size_per_machine; ++i)
        free(X_batch[i]);
    free(X_batch);
    free(Y_batch);
    free(temp_values);
    totalTime = MPI_Wtime() - totalTime;
    if (machine_id == 0)
    {
        printf("\nCommunication Time: %.3f\n", comTime);
        printf("Total Time (T_w_com): %.3f\n", totalTime);
        printf("Total Time (T_wo_com): %.3f\n\n", totalTime - comTime);

        // open file
        FILE *linear_graph_nT = NULL;
        linear_graph_nT = fopen("linear_graph_nT.txt", "a");
        if (linear_graph_nT == NULL)
        {
            fprintf(stderr, "Error opening output file linear_graph_nT\n");
            MPI_Finalize();
            exit(-1);
        }
        else
        {
            printf("opening output file linear_graph_nT\n");
        }
        fprintf(linear_graph_nT, "%d, ", n_samples);           // so luong mau
        fprintf(linear_graph_nT, "%f, ", totalTime);           // t_w_comm
        fprintf(linear_graph_nT, "%f\n", totalTime - comTime); // t_wo_comm:
        fclose(linear_graph_nT);

        // open file
        FILE *linear_graph_nCPUT = NULL;
        linear_graph_nCPUT = fopen("linear_graph_nCPUT.txt", "a");
        if (linear_graph_nCPUT == NULL)
        {
            fprintf(stderr, "Error opening output file linear_graph_nCPUT\n");
            MPI_Finalize();
            exit(-1);
        }
        else
        {
            printf("opening output file linear_graph_nCPUT\n");
        }
        fprintf(linear_graph_nCPUT, "%d, ", n_machines);          // so luong mau
        fprintf(linear_graph_nCPUT, "%f, ", totalTime);           // t_w_comm
        fprintf(linear_graph_nCPUT, "%f\n", totalTime - comTime); // t_wo_comm:
        fclose(linear_graph_nCPUT);
    }
    return 0;
}

void timestamp(void)
{
#define TIME_SIZE 40

    static char time_buffer[TIME_SIZE];
    const struct tm *tm;
    size_t len;
    time_t now;

    now = time(NULL);
    tm = localtime(&now);

    len = strftime(time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm);

    printf("%s\n", time_buffer);

    return;
#undef TIME_SIZE
}

void shuffle(int *array, int n)
{
    if (n > 1)
    {
        int i;
        for (i = 0; i < n - 1; i++)
        {
            int j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}