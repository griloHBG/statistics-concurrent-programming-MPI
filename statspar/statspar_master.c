//Henrique Borges Garcia Nº USP 6882298
/******************************************************************
Problema:
    Utilizando uma máquina de memória distribuída (cluster com várias máquinas de memórica local compartilhada), realizar a paralelização de um programa que calcula média, média harmônica, mediana, moda, variância, desvio padrão e coeficiente de variação das colunas de uma matriz gerada aleatóriamente a partir da quantidade de linhas e colunas passadas via linha de comando, cujos valores double são obtido com um gerador de sequência numérica pseudo-aleatória com semente também passada via linha de comando (juntamente com as quantidades de linhas e colunas). É importante lembrar que, para o cálculo da mediana, cada uma das colunas precisam ser ordenadas.

Particionamento:
    Por dados. Cada tarefa calculará todas as estatísticas de cada coluna da matriz de dados, logo serão C tarefas, sendo C a quantidade de colunas da matriz de entrada, sendo que matriz utilizada para cáclulo das estatísticas será gerada pela tarefa zero.
    Cada uma dessas tarefas terão um particionamento local por função, em sub-tarefas, em que cada sub-tarefa calculará uma estatística distinta, logo serão 7 sub-tarefas. A sub-tarefa da mediana, que necessita dos valores ordenadas, realizará seus cálculos com uma cópia da coluna original, que será ordenada pela própria sub-tarefa.

Comunicação:
    Antes de se iniciar qualquer cálculo, a matriz (presente na tarefa zero) será distribuída entre todas as tarefas. Assim, cada tarefa calculará todas as estatísicas necessárias e, ao fim, a tarefa zero concatenará os dados de cada tarefa para imprimí-los na tela.
    As sub-tarefas que calculam média, média harmônica, mediana (aqui já incluindo a ordenação da coluna) e moda são totalmente independentes, logo não precisam realizar comunicação. A sub-tarefa da variância depende do resultado da sub-tarefa da média, a do desvio padrão depende da variância e a do coeficiente de variação depende do desvio padrão e da média. Logo, essas sub-tarefas que apresentam dependência precisam comunicar-se para uma passar o resultado que a outra precisa.

Aglomeração:
    Da maneira que o PCAM está sendo conduzido, a etapa de Aglomeração não precisa ser aplicada, já que as tarefas criadas serão justamente as implementadas.
    As sub-terefas que apresentam dependência serão aglomeradas em uma única sub-tarefa. As outras sub-tarefas ocorrerão em paralelo com esta, ou seja, uma seção para cada sub-tarefa independente e uma única seção para as sub-tarefas dependentes.

Mapeamento:
    O escalonamento das tarefas será realizado pela MPI e o escalonamento das sub-tarefas será realizado pelo Sistema Operacional de cada nó do cluster.

Como compilar:
    $ mpicc statspar_master.c -o statspar_master -fopenmp -lm
    $ mpicc statspar_slave.c -o statspar_slave -fopenmp -lm

Como utilizar o executável:

    Para EXECUTAR:

    mpirun -np 1 statspar_master LIN COL SEED

    onde:
    LIN é a quantidade de linhas da matriz a ser gerada
    COL é a quantidade de COLUNAS da matriz a ser gerada
    SEED é a semente utilizada na geração pseudo-aleatória da matriz

    statspar_slave é executado pelo master, e ambos devem estar na mesma pasta!

******************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#undef EXEMPLO
#undef VERBOSE

#ifdef EXEMPLO
double test_matriz[] = {9, 8, 4, 5,
                        4, 12, 20, 40,
                        8, 8, 4, 4,
                        8, 12, 4, 21,
                        33, 44, 20, 1,
                        10, 18, 17, 10,
                        };
#endif

int main(int argc, char* argv[])
{
    int my_rank, num_procs;
    int errcodes[10];
    int root = 0;
    double *matriz, *mediana, *media, *media_har, *moda, *variancia, *dp, *cv; //Define a matriz (forma linear), vetores de medidas estatísticas
    int lin, col, i, j; //Define as variáveis de índices e dimensões
    char program_name[50];
    char** slaves_argv;
    int matrizColumnSize;

    MPI_Comm    inter_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    strcpy(program_name, argv[0]);

    if (argc < 3) { //se os argumentos de linha de comando não estiverem corretos
        printf("\nUsage:\n\n%s <LIN> <COL> <SEED>\n\nWhere:\n"
               "\tLIN is the amount of lines of the generated matrix\n"
               "\tCOL is the amount of columns of the generated matrix\n"
               "\tSEED is the seed for the pseudo-random matrix generation\n\n", argv[0]);
        MPI_Finalize();
        exit(0);
    }



#ifndef EXEMPLO
    int seed;
    lin = atoi(argv[1]);
    col = atoi(argv[2]);
    seed = atoi(argv[3]);
    srand(seed);
#else
    lin = 6;
    col = 4;
#endif

    slaves_argv = (char**) malloc(1 * sizeof(char*));
    slaves_argv[0] = (char*) malloc(10 * sizeof(char));

#ifdef VERBOSE
    printf("creating slaves argv!\n");
#endif


    sprintf(slaves_argv[0], "%d", lin);

#ifdef VERBOSE
    printf("spawning %d slaves!\n", col);
#endif

    if(MPI_Comm_spawn("statspar_slave", slaves_argv, col, MPI_INFO_NULL, root, MPI_COMM_WORLD, &inter_comm, errcodes) != MPI_SUCCESS)
        for(i = 0; i< 10; i++)
            printf("%d | ",errcodes[i]);


#ifdef VERBOSE
    printf("%d slaves already spawned!\n", col);
#endif

    matriz = (double *) malloc(lin * col * sizeof(double)); //Aloca a matriz


#ifdef VERBOSE
    #ifdef EXEMPLO
        printf("\tlin\t%d\tcol\t%d\n", lin, col);
    #else
        printf("\tlin\t%d\tcol\t%d\tseed\t%d\n", lin, col, seed);
    #endif
#endif

    for(i = 0; i < lin; i++)
    {
        for(j = 0; j < col; j++)
        {
#ifndef EXEMPLO
            matriz[i*col+j] = (int)(50*((rand())/(1.0 * RAND_MAX)));
#else
            matriz[i*col+j] = test_matriz[i*col+j];
#endif
#ifdef VERBOSE
            printf("%10.2lf", matriz[i*col+j]);
#endif
        }
#ifdef VERBOSE
        printf("\n");
#endif
    }

    MPI_Datatype doubleColunaMatriz_dt;
    MPI_Type_vector(lin, 1, col, MPI_DOUBLE, &doubleColunaMatriz_dt);
    MPI_Type_commit(&doubleColunaMatriz_dt);
    MPI_Type_size(doubleColunaMatriz_dt, &matrizColumnSize);
    MPI_Type_create_resized(doubleColunaMatriz_dt, 0, 1 * sizeof(double), &doubleColunaMatriz_dt);
    MPI_Type_commit(&doubleColunaMatriz_dt);

    MPI_Scatter(matriz, 1, doubleColunaMatriz_dt, matriz, 6, MPI_DOUBLE, MPI_ROOT, inter_comm);

#pragma omp parallel default(none) shared(col, mediana, media, media_har, moda, variancia, dp, cv)
    {
#pragma omp sections
        {
#pragma omp section
            media = (double *) malloc(col * sizeof(double));
#pragma omp section
            media_har = (double *) malloc(col * sizeof(double));
#pragma omp section
            mediana = (double *) malloc(col * sizeof(double));
#pragma omp section
            moda = (double *) malloc(col * sizeof(double));
#pragma omp section
            variancia = (double *) malloc(col * sizeof(double));
#pragma omp section
            dp = (double *) malloc(col * sizeof(double));
#pragma omp section
            cv = (double *) malloc(col * sizeof(double));
        }
    }

#ifdef VERBOSE
    printf("gathering media!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, media, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);
#ifdef VERBOSE
    printf("gathering media_har!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, media_har, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);
#ifdef VERBOSE
    printf("gathering mediana!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, mediana, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);
#ifdef VERBOSE
    printf("gathering moda!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, moda, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);
#ifdef VERBOSE
    printf("gathering variancia!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, variancia, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);
#ifdef VERBOSE
    printf("gathering dp!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, dp, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);
#ifdef VERBOSE
    printf("gathering cv!\n");
#endif
    MPI_Gather(NULL, 1, MPI_DOUBLE, cv, 1, MPI_DOUBLE, MPI_ROOT, inter_comm);

    for (i = 0; i < col; i++)
        printf("%.1lf ", media[i]);
    printf("\n");
    for (i = 0; i < col; i++)
        printf("%.1lf ", media_har[i]);
    printf("\n");
    for (i = 0; i < col; i++)
        printf("%.1lf ", mediana[i]);
    printf("\n");
    for (i = 0; i < col; i++)
        printf("%.1lf ", moda[i]);
    printf("\n");
    for (i = 0; i < col; i++)
        printf("%.1lf ", variancia[i]);
    printf("\n");
    for (i = 0; i < col; i++)
        printf("%.1lf ", dp[i]);
    printf("\n");
    for (i = 0; i < col; i++)
        printf("%.1lf ", cv[i]);
    printf("\n");

    free(matriz); //Desaloca a matriz
    free(media); //Desaloca o vetor de media
    free(media_har); //Desaloca o vetor de media_har
    free(mediana); //Desaloca vetor de mediana
    free(moda); //Desaloca vetor de moda
    free(variancia);  //Desaloca vetor de variância
    free(dp); //Desaloca vetor de desvio padrão
    free(cv); //Desaloca vetor de coeficiente de variação
    free(slaves_argv[0]);
    free(slaves_argv[1]);
    free(slaves_argv);

    MPI_Finalize();
}