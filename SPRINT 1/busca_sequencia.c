// LUAN FÁBIO MARINHO GALINDO


int busca_sequencia123(int array10000[9999]) 
{
    int cont = 0;
    for (int i = 0; i < 9998; i++)
        if (array10000[i] == 1 && array10000[i + 1] == 2 && array10000[i + 2] == 3)
            cont ++;
    
    return cont;
}

