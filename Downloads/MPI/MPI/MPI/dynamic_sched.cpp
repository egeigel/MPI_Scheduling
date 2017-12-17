#include <iostream>
#include <ratio>
#include <chrono>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>


#define ARR_START_SEND 100
#define ARR_START_RECV 100
#define ARR_END_SEND 101
#define ARR_END_RECV 101
#define RESULT_TAG 1000
#define MASTER 0

#ifdef __cplusplus
extern "C" {
#endif

    float f1(float x, int intensity);
    float f2(float x, int intensity);
    float f3(float x, int intensity);
    float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

float (*function) (float,int);

int main (int argc, char* argv[])
{
    if (argc < 6)
    {
        std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
        return -1;
    }
    int a = atoi(argv[2]);
    int n = atoi(argv[4]);
    int intensity = atoi(argv[5]);
    float multiplier=(atoi(argv[3])-a)/(float)n;
    double integralPartial = 0.0;
    MPI_Comm comm;

    switch(atoi(argv[1])){
        case 1:
            function = &f1;
            break;
        case 2:
            function = &f2;
            break;
        case 3:
            function = &f3;
            break;
        case 4:
            function = &f4;
            break;
        default:
            std::cerr<<"Invalid function number provided\n";
            exit(-1);
    }

    int rank, size;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double globalResult = 0.0;
    int master = 0;
    int chunkSize = n/(size);
    int arrIndex = 0;
    double receivedResult = 0.0;
    int startIndex, endIndex;
    MPI_Status status;

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    if(rank == 0){
        for(int i=1;i<size;i++){
		//send start index to processes
            MPI_Send(&arrIndex,1,MPI_INT,i,ARR_START_SEND,MPI_COMM_WORLD);    

            arrIndex += chunkSize;
            if(arrIndex >= n){
                arrIndex = n;
            }
		//send end index to processes
            MPI_Send(&arrIndex,1,MPI_INT,i,ARR_END_SEND,MPI_COMM_WORLD);    
        }
        sleep(1);
        for(int i=0;i<size;){
		//receive result from each rank 
            MPI_Recv(&receivedResult,1,MPI_DOUBLE_PRECISION,MPI_ANY_SOURCE,RESULT_TAG,MPI_COMM_WORLD,&status);               
		//add it to local result
            integralPartial += receivedResult;
            if(arrIndex >= n){
		//send size to processes
                MPI_Send(&n,1,MPI_INT,status.MPI_SOURCE,ARR_START_SEND,MPI_COMM_WORLD);    
                i++;
            }
            else{
		//send start index of new chunk
                MPI_Send(&arrIndex,1,MPI_INT,status.MPI_SOURCE,ARR_START_SEND,MPI_COMM_WORLD);    
                arrIndex += chunkSize;
                if(arrIndex >= n){
                    arrIndex = n;
                }
		//sned end index of new chunk
                MPI_Send(&arrIndex,1,MPI_INT,status.MPI_SOURCE,ARR_END_SEND,MPI_COMM_WORLD);    
            }
        }
        std::cout<<integralPartial<<std::endl;
    }
    else{
        while(true){
		//keep receiving start indexs until integration is complete
            MPI_Recv(&startIndex,1,MPI_INT,MASTER,ARR_START_RECV,MPI_COMM_WORLD,&status);
            if(startIndex == n){
                double scapegoat = 0.0;
                MPI_Send(&scapegoat,1,MPI_DOUBLE_PRECISION,MASTER,RESULT_TAG,MPI_COMM_WORLD); 
                break;
            }
		//keep receiving end indexs until integration is complete
            MPI_Recv(&endIndex,1,MPI_INT,MASTER,ARR_END_RECV,MPI_COMM_WORLD,&status); 

            double result = 0.0;
            for (int x=startIndex;x<endIndex;x++){
                result += (double)function(a + (x + 0.5) * multiplier, intensity) * multiplier;
            }

            // send result to master node
            MPI_Send(&result,1,MPI_DOUBLE_PRECISION,MASTER,RESULT_TAG,MPI_COMM_WORLD); 
        }
    }
    MPI_Finalize();
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    if(rank == 0){
        std::cerr<<elapsed_seconds.count()<<std::endl;
    }
    return 0;
}
