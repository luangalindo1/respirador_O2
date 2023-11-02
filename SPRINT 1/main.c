// LUAN FÁBIO MARINHO GALINDO


#include <stdio.h>
#include <stdlib.h>
//#include <locale.h>
#include <time.h>
#include "busca_sequencia.h"

int main()
{
    //setlocale(LC_ALL, "PORTUGUESE");
    int vet [9999], i;
    
    srand(time(NULL)); // Garantir que os valores pseudo-aleatórios mudem a cada execução
    for (i = 0; i < 10000; i++)
        vet[i] = rand() % 9;
    
    printf("\n A Sequência ocorreu %d vezes.", busca_sequencia123(vet));
    
    return 0;
}
