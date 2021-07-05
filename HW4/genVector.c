#include <stdio.h>
typedef int dtype;
const int N  = 5;
dtype VECTOR[5]={1, 2, 3, 4, 5};

int main(){
    int i;
    FILE* pFile;  
    pFile = fopen("input_vector" , "wb");
    printf("Generation completed!\n");
    fwrite(&N, sizeof(int), 1, pFile);
    for (i = 0; i<N; i++){
        fwrite(&VECTOR[i], sizeof(dtype), 1, pFile);
    }
    fclose(pFile);
    // int m, n;
    // pFile = fopen("input_Matrix" , "r");
    // fread(&m ,  sizeof(int) , 1 ,pFile);
    // fread(&n ,  sizeof(int) ,1 , pFile);
    // printf("m: %d, n: %d\n", n, n);
    // int data;
    // for(r = 0; r<m; r++){
    //     for(c =0; c<n; c++){
    //         fread(&data, sizeof(int), 1, pFile);
    //         printf("%d ", data);
    //     }
    // }
    // fclose(pFile);
    return 0;
}