#include <stdio.h>

const int M  = 5;
const int N  = 5;
int MATRIX[5][5]={
    {1, 0, 0, 1, 1},
    {0, 0, 0, 1, 1},
    {0, 0, 0, 0, 1},
    {1, 1, 1, 0, 1},
    {0, 0, 0, 0, 1}
};

int main(){
    int r, c;
    FILE* pFile;  
    pFile = fopen("input_Matrix" , "wb");
    printf("Generation completed!\n");
    fwrite(&M, sizeof(int), 1, pFile);
    fwrite(&N, sizeof(int), 1, pFile);
    for (r= 0; r<M; r++){
        for(c = 0; c<N; c++){
            fwrite(&MATRIX[r][c], sizeof(int), 1, pFile);
        }
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