#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <string>
using namespace std;


#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);

  
#ifdef __cplusplus
}
#endif

void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    /* create temp arrays */
    int L[n1], R[n2];
 
    /* Copy data to temp arrays L[] and R[] */
#pragma omp parallel for schedule(runtime)
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
#pragma omp parallel for schedule(runtime)
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
 
    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }
 
    /* Copy the remaining elements of R[], if there
       are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}


void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l+(r-l)/2;
 
        // Sort first and second halves
	#pragma omp parallel
	{
	#pragma omp single
	{	
		#pragma omp task
        	mergeSort(arr, l, m);

		#pragma omp task
        	mergeSort(arr, m+1, r);
	}
	}
        merge(arr, l, m, r);
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

  int * arr = new int [atoi(argv[1])];

  generateMergeSortData (arr, n);
  omp_set_schedule(omp_sched_static , -1);
  omp_set_num_threads(atoi(argv[2]));

  for (int i =0;i<n;i++){ 
        cout<<"[ "<<arr[i]<<" ]"<<endl;
   }



  mergeSort(arr , 0 , n-1);

/*
--------------------------
   for (int i =0;i<n;i++){
	cout<<"[ "<<arr[i]<<" ]"<<endl;
   }

  //write code here
  for (int i=2;i<n/2;i+=i){
		for (int j=0;j<n;j+i){
			int index=j;
			cout<<index<<endl;
			for (int k = j ; k<j+i;k++){
				for (int m=j; m<j+i;j++){
					if(arr[k]>arr[m]){
						index=m;
						cout<<"moving "<<k<<" to "<<m<<endl;
					}
				}
				buffer[index] = arr[k];
			}
		}
	#pragma omp parallel for schedule(runtime)
		for (int i=0;i<n;i++){
			arr[i]=buffer[i];
		}


    for (int i =0;i<n;i++){ 
        cout<<"[ "<<arr[i]<<" ]"<<endl;
   }

  }
*/
  for (int i =0;i<n;i++){ 
        cout<<"[ "<<arr[i]<<" ]"<<endl;
   }


  checkMergeSortResult (arr, atoi(argv[1]));
  
  delete[] arr;

  return 0;
}
