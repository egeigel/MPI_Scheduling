#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <ctime>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

  void generatePrefixSumData (int* arr, size_t n);
  void checkPrefixSumResult (int* arr, size_t n);
  
  
#ifdef __cplusplus
}
#endif


void prefixSumPartial(int * pr , int * arr , int start , int modFlag , int delta , int n){
	int total_threads = omp_get_num_threads();

	if(start==0){
         	pr[start] = 0;
         }else{
                pr[start*delta] = arr[(start*delta)-1];
         }
	for (int i=(start*delta)+1;i<(start+1)*delta;i++){
		cout<<"in iteration "<<i<<endl;
		 pr[i] =  pr[i-1] + arr[i-1];
	         if (i==((delta*total_threads)-1) && modFlag!=0){
        	 	for(int j=delta*total_threads;j<n;j++){
				cout<<"in iteration "<<j<<endl; 
        	        	pr[j] = pr[j-1] + arr[j-1];
        	        }
        	 }
	}
}


int main (int argc, char* argv[]) {

  //forces openmp to create the threads beforehand
#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }
  }
  
  if (argc < 3) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }
  int n = atoi(argv[1]);
  int * arr = new int [n];
  generatePrefixSumData (arr, n);
  /*for (int i=0;i<n;i++){
	arr[i]=i+1;
  }*/

  //write code here
  omp_set_num_threads(atoi(argv[2]));
  
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  
  n = n + 1;
  int * pr = new int [n];
  int total_threads;
  int delta;
  int modFlag=0;

  omp_set_schedule(omp_sched_static, -1);

#pragma omp parallel shared(pr,arr)
{
	total_threads = omp_get_num_threads();
	#pragma omp single
	{
		delta = n / total_threads;
		if(n%total_threads!=0){
			modFlag=1;
		}
		else{
			modFlag=0;
		}
		for (int i=0;i<delta;i++){
			#pragma omp task
			cout<<"in loop "<<i<<endl;
			prefixSumPartial(pr , arr , i , modFlag , delta , n);
		

			/*
			if
				if(i==0){
					pr[i] = 0;
				}else{
					pr[i] = arr[i-1];
				}
			}
			else{
				pr[i] =  pr[i-1] + arr[i-1];
				if (i==((delta*total_threads)-1) && modFlag!=0){
        		        	for(int j=delta*total_threads;j<n;j++){
        		        	        pr[j] = pr[j-1] + arr[j-1];
        		        	}
        			}
	
			}*/
		}
		#pragma omp taskwait
		cout<<"ending for loop "<<endl;
	}
}
for(int i=0;i<n;i++){
cout<<"[ "<<pr[i]<<" ]"<<endl;
}

  for(int j=2;j<total_threads+1; j++){
	if(modFlag!=0 && j==total_threads){
		pr[n-1] += pr[((j-1)*(delta))-1];
	}
	else{
		pr[((j)*(delta))-1] += pr[((j-1)*(delta))-1];

	}
  }



#pragma omp parallel shared(pr,arr)
{
	int t_id = omp_get_thread_num();
	int total_threads = omp_get_num_threads();
	int delta = n / total_threads;
	int modFlag = 0;
	int mod = n%total_threads;
	if(n%total_threads!=0){
		modFlag=1;
	}
	for (int k=(t_id+1)*(delta);k<((t_id+2)*(delta))-1 && k<n-mod ; k++){
		if(modFlag!=0 && k==(delta*total_threads)-2){
			pr[k]+= pr[((t_id+1)*(delta))-1];
			for(int m=k+1;m<n-1;m++){
				pr[m] += pr[((t_id+1)*(delta))-1];
			}
		}
		else{
			pr[k]+= pr[((t_id+1)*(delta))-1];

		}
	}
}


  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;
  
  std::cerr<<elapsed_seconds.count()<<std::endl;

  checkPrefixSumResult(pr, atoi(argv[1]));
  
  delete[] arr;
  delete[] pr;

  return 0;
}
