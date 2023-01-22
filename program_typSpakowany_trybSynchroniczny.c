#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"


struct npc{
    int id;
    int health;
    char name[10];
    char type[10];
};

int main(int argc, char **argv)
{
    int i;
    int rank, ranksent, size, dest, tag, nextrank, prevrank;
    MPI_Aint extent;
    MPI_Datatype npc_type;
    double start, stop;
    struct npc f1, f2;

    MPI_Aint offsets[4];

    f1.id = 1;
    f1.health = 100;
    strcpy(f1.name, "Mietek");
    strcpy(f1.type, "obywatel");

    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if(rank == size - 1){
        prevrank = rank - 1;
        nextrank = 0;}
    else if(rank == 0){
        prevrank = size - 1;
        nextrank = rank + 1;}
    else{
        prevrank = rank - 1;
        nextrank = rank + 1;}

    offsets[0] = 0;

    MPI_Type_extent(MPI_INT, &extent);
    offsets[1] = extent;

    MPI_Type_extent(MPI_INT, &extent);
    offsets[2] = offsets[1] + extent;

    MPI_Type_extent(MPI_CHAR, &extent);
    offsets[3] = offsets[2] + 10 * extent;

    int blockcounts[] = {1,1,10,10};
    MPI_Datatype oldtypes[] = {MPI_INT,MPI_INT,MPI_CHAR,MPI_CHAR};

    MPI_Type_create_struct(4, blockcounts, offsets, oldtypes, &npc_type);
    MPI_Type_commit(&npc_type);

    if(rank == 0){
        start = MPI_Wtime();

        int buffer_size = 100;
        void* buffer = malloc(buffer_size);
        int position = 0;

        MPI_Pack(&f1.id, 1, MPI_INT, buffer, buffer_size, &position, MPI_COMM_WORLD);
        MPI_Pack(&f1.health, 1, MPI_INT, buffer, buffer_size, &position, MPI_COMM_WORLD);
        MPI_Pack(f1.name, 10, MPI_CHAR, buffer, buffer_size, &position, MPI_COMM_WORLD);
        MPI_Pack(f1.type, 10, MPI_CHAR, buffer, buffer_size, &position, MPI_COMM_WORLD);
        MPI_Ssend(buffer, position, MPI_PACKED, nextrank, 0, MPI_COMM_WORLD);
        position = 0;
        MPI_Recv(buffer, buffer_size, MPI_PACKED, prevrank, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Unpack(buffer, buffer_size, &position, &f2.id, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, buffer_size, &position, &f2.health, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, buffer_size, &position, f2.name, 10, MPI_CHAR, MPI_COMM_WORLD);
        MPI_Unpack(buffer, buffer_size, &position, f2.type, 10, MPI_CHAR, MPI_COMM_WORLD);

        stop = MPI_Wtime();

        printf("rank = %d :\n\tid = %d\n\thealth = %d\n\tname = %s\n\ttype = %s\n",
               rank, f2.id, f2.health, f2.name, f2.type);
        printf("time = %f\n", stop - start );

        FILE* wfp = fopen("./dane_3.csv", "a+");
        fprintf(wfp, "%d,%lf\n", size, stop - start);}

    else{
        MPI_Recv(&f2,1,npc_type,prevrank,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        MPI_Send(&f2,1,npc_type,nextrank,0,MPI_COMM_WORLD);}

    MPI_Finalize();

    return(0);
}