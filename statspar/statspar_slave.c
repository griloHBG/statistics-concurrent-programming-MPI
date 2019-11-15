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
#include <math.h>

//Quicksort adaptado de //https://www.geeksforgeeks.org/quick-sort/
int partition (double *arr, int low, int high, int C){
    int i, j;
    double pivot,swap;

    // pivot (Element to be placed at right position)
    pivot = arr[high*C];

    i = (low - 1);  // Index of smaller element

    for (j = low; j <= high-1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j*C] <= pivot)
        {
            i++;    // increment index of smaller element

            // swap arr[i] and arr[j]
            swap = arr[i*C];
            arr[i*C] = arr[j*C];
            arr[j*C] = swap;
        }
    }

    //swap arr[i + 1] and arr[high]
    swap = arr[(i + 1)*C];
    arr[(i + 1)*C] = arr[high*C];
    arr[high*C] = swap;

    return (i + 1);

} // fim partition


/* low  --> Starting index,  high  --> Ending index */
void quicksort(double *arr, int low, int high, int C){
    int pi;

    if (low < high)  {
        /* pi is partitioning index, arr[pi] is now
           at right place */
        pi = partition(arr, low, high, C);

        quicksort(arr, low, pi - 1, C);  // Before pi
        quicksort(arr, pi + 1, high, C); // After pi
    }

} // fim quicksort

/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot
   https://www.geeksforgeeks.org/quick-sort/
*/

void ordena_colunas(double *matriz, int lin, int col) {
    int j;

    for (j = 0; j < col; j++) {
        //manda o endereco do primeiro elemento da coluna, limites inf e sup e a largura da matriz
        quicksort(&matriz[j], 0, lin-1, col);
    }
}

void calcula_media(double *matriz, double *vet, int lin, int col){
    int i,j;
    double soma;
    for(i=0;i<col;i++){
        soma=0;
        for(j=0;j<lin;j++){
            soma+=matriz[j*col+i];
        }
        vet[i]=soma/j;
    }
}

void calcula_media_harmonica(double *matriz, double *vet, int lin, int col){
    int i,j;
    double soma;
    for(i=0;i<col;i++){
        soma=0;
        for(j=0;j<lin;j++){
            soma+=(1/(matriz[j*col+i]));
        }
        vet[i]=lin/soma;
    }
}

void calcula_mediana(double *matriz, double *vet, int lin, int col) {
    int j;
    for (j = 0; j < col; j++) {
        vet[j] = matriz[((lin/2)*col)+j];
        if(!(lin%2))  {
            vet[j]+=matriz[((((lin-1)/2)*col)+j)];
            vet[j]*=0.5;
        }
    }
}

//Adaptado de https://www.clubedohardware.com.br/forums/topic/1291570-programa-em-c-que-calcula-moda-media-e-mediana/
double moda_aux(double *matriz,int lin){
    int i, j;
    double *cont;
    cont=(double*)malloc(lin*sizeof(double));
    float conta=0, moda;

    for(i=0;i<lin;i++){
        for(j=i+1;j<lin;j++){

            if(matriz[i]==matriz[j]){
                cont[i]++;
                if(cont[i]>conta){
                    conta=cont[i];
                    moda=matriz[i];
                }
            }

        }
        cont[i]=0;
    }
    free(cont);
    if(conta == 0){
        return -1;
    }
    else{
        return moda;
    }

}


void calcula_moda(double *matriz,double *moda,int lin, int col){
    int i,j;
    double *aux=(double *)malloc(lin*sizeof(double));
    for(i=0;i<col;i++){
        for(j=0;j<lin;j++)
        {
            aux[j]=matriz[j*col+i]; //Faz a transposta da linha para coluna
        }
        moda[i]=moda_aux(aux,lin);
    }
    free(aux);

}

void calcula_variancia(double *matriz, double *media,double *variancia, int lin, int col)
{
    int i,j;
    double soma;
    for(i=0;i<col;i++){
        soma=0;
        for(j=0;j<lin;j++){
            soma+=pow((matriz[j*col+i]-media[i]),2);
        }
        variancia[i]=soma/(lin-1);
    }
}

void calcula_desvio_padrao(double *variancia,double *dp, int col)
{
    int i;
    for(i=0;i<col;i++){
        dp[i]=sqrt(variancia[i]);
    }
}

void calcula_coeficiente_variacao(double *media,double *dp,double *cv, int col)
{
    int i;
    for(i=0;i<col;i++){
        cv[i]=dp[i]/media[i];
    }
}

int main(int argc, char* argv[])
{
    int my_rank, num_procs;
    int root = 0;

    MPI_Comm    inter_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    MPI_Comm_get_parent(&inter_comm);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int processor_name_len;

    MPI_Get_processor_name(processor_name, &processor_name_len);

    printf("-Running on %s, rank %2d-\n", processor_name, my_rank);

    //printf("[slv%d] is in!\n", my_rank);

    char program_name[50];
    strcpy(program_name, argv[0]);

    int lin, col=1; //Define as variáveis de índices e dimensões


    double *coluna, mediana, media, media_har, moda, variancia, dp, cv; //Define a coluna (forma linear), vetores de medidas estatísticas

    /*if (argc < 2) { //se os argumentos de linha de comando não estiverem corretos
        printf("This program (called %s) must be executed by its master, peasant.\n", argv[0]);
        exit(0);
    }*/

    lin = atoi(argv[1]);

    coluna = (double *) malloc(lin * sizeof(double)); //Aloca a coluna

    MPI_Scatter(coluna, lin, MPI_DOUBLE, coluna, lin, MPI_DOUBLE, root, inter_comm);


#pragma omp parallel sections default(none) shared(coluna, lin, col, media, media_har, mediana, moda, variancia, dp, cv)
    {
#pragma omp section
        {
            calcula_media(coluna, &media, lin, col);

            calcula_variancia(coluna, &media, &variancia, lin, col);

            calcula_desvio_padrao(&variancia, &dp, col);

            calcula_coeficiente_variacao(&media, &dp, &cv, col);
        }

#pragma omp section
        calcula_media_harmonica(coluna, &media_har, lin, col);

#pragma omp section
        {
            double* matriz_ordenada = (double *) malloc(lin * col * sizeof(double)); //Aloca a matriz
            memcpy(matriz_ordenada, coluna, lin * col * sizeof(double));
            ordena_colunas(matriz_ordenada, lin, col);
            calcula_mediana(matriz_ordenada, &mediana, lin, col);
        }

    }

    MPI_Gather(&media, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);
    MPI_Gather(&media_har, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);
    MPI_Gather(&mediana, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);
    MPI_Gather(&moda, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);
    MPI_Gather(&variancia, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);
    MPI_Gather(&dp, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);
    MPI_Gather(&cv, 1, MPI_DOUBLE, NULL, 1, MPI_DOUBLE, root, inter_comm);

    free(coluna);
    MPI_Finalize();

    //printf("[slv%d] is out!\n", my_rank);
}