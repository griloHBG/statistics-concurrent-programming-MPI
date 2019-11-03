//compilar:
//gcc statsseq.c -lm -o statsseq
//executar:
//./statsseq <LIN> <COL> <SEED>

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

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


int main(int argc,char **argv){
    int lin,col,i,j; //Define as variáveis de índices e dimensões
    double *matriz,*mediana,*media,*media_har,*moda,*variancia,*dp,*cv; //Define a matriz (forma linear), vetores de medidas estatísticas

    if (argc < 3) { //se os argumentos de linha de comando não estiverem corretos
        printf("\nUsage:\n\n%s <LIN> <COL> <SEED>\n\nWhere:\n"
               "\tLIN is the amount of lines of the generated matrix\n"
               "\tCOL is the amount of columns of the generated matrix\n"
               "\tSEED is the seed for the pseudo-random matrix generation\n\n", argv[0]);
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

    matriz=(double *)malloc(lin*col * sizeof(double)); //Aloca a matriz
    media=(double *)malloc(col * sizeof(double)); //Aloca o vetor de media
    media_har=(double *)malloc(col * sizeof(double)); //Aloca o vetor de media harmônica
    mediana=(double *)malloc(col * sizeof(double)); //Aloca o vetor de mediana
    moda=(double *)malloc(col * sizeof(double)); //Aloca o vetor de moda
    variancia=(double *)malloc(col * sizeof(double)); //Aloca o vetor de variância
    cv=(double *)malloc(col * sizeof(double)); //Aloca o vetor de coeficiente de variação
    dp=(double *)malloc(col * sizeof(double)); //Aloca o vetor de desvio padrão

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

    calcula_media(matriz,media,lin,col);
    calcula_media_harmonica(matriz,media_har,lin,col);
    ordena_colunas(matriz,lin,col);
    calcula_mediana(matriz,mediana,lin,col);
    calcula_moda(matriz,moda,lin,col);
    calcula_variancia(matriz,media,variancia,lin,col);
    calcula_desvio_padrao(variancia,dp,col);
    calcula_coeficiente_variacao(media,dp,cv,col);
    for(i=0;i<col;i++)
        printf("%.1lf ",media[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",media_har[i]);
    printf("\n");
    for(i=0;i<col;i++)
       printf("%.1lf ",mediana[i]);
    printf("\n");
    for(i=0;i<col;i++)
       printf("%.1lf ",moda[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",variancia[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",dp[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",cv[i]);
    printf("\n");
    free(matriz); //Desaloca a matriz
    free(media); //Desaloca o vetor de media
    free(media_har); //Desaloca o vetor de media_har
    free(mediana); //Desaloca vetor de mediana
    free(moda); //Desaloca vetor de moda
    free(variancia);  //Desaloca vetor de variância
    free(dp); //Desaloca vetor de desvio padrão
    free(cv); //Desaloca vetor de coeficiente de variação
}