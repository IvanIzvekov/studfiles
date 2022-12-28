#include <iostream>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <omp.h>

using namespace std;

const int n = 1000;

double wtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}


struct parametr{
    vector<vector<double>> f;
    vector<vector<double>> s;
    int L;
    int R;
};
vector<vector<double>> result2(n, vector<double>(n, 0));

void* matrix_parallel(void* par)
{
    double r;
    parametr param = *(parametr*)par;
    vector<vector<double>> first(param.f);
    vector<vector<double>> second(param.s);
    int L = param.L;
    int R = param.R;
    for (int i = L; i < R; i++)
    {
        for (int k = 0; k < n; k++) 
        {
            r = first[i][k];
            for (int j = 0; j < n; j++)
                result2[i][j] += r * second[k][j];
        }
    }
    return nullptr;
}



double doing_parallel(vector<vector<double>> first, vector<vector<double>> second,int num_of_threads)
{
    double start = wtime();
    int num = n/num_of_threads;
    vector<parametr> par(num_of_threads);
    vector<pthread_t> id(num_of_threads);
    for(int i=0;i<num_of_threads;i++)
    {
        int L = i*num;
        int R = (i+1)*num;
        if(R<n && i==num_of_threads-1)
            R=n;
        
        par[i].f = first;
        par[i].s = second;
        par[i].L = L;
        par[i].R = R;

	    pthread_create(&id[i], NULL, &matrix_parallel, &par[i]);

    }
    for(int i=0;i<num_of_threads;++i)
        pthread_join(id[i],nullptr);
    double end = wtime();
    return end - start;
}

vector<vector<double>> matrix(vector<vector<double>> first, vector<vector<double>> second)
{

    vector<vector<double>> result (first.size(), vector<double>(first.size(), 0));
    #pragma omp parallel for
    for (int k = 0; k < first.size(); ++k)
        for (int i = 0; i < first.size(); ++i)
            for (int j = 0; j < first.size(); ++j)
                    result[i][k] += first[i][j] * second[j][k];
    return result;
}
vector<vector<double>> matrix2(vector<vector<double>> first, vector<vector<double>> second)
{
    double r;
    vector<vector<double>> result(first.size(), vector<double>(first.size(), 0));
    #pragma omp parallel for
    for (int i = 0; i < first.size(); i++)
    {
        for (int k = 0; k < first.size(); k++) 
        {
            r = first[i][k];
            for (int j = 0; j < first.size(); j++)
                result[i][j] += r * second[k][j];
        }
    }
    return result;
}

void fill(vector<vector<double>>& first)
{
    #pragma omp parallel for
    for (auto& i : first)
        for (auto& j : i)
            j = (double)rand() / RAND_MAX;

}

void print(vector<vector<double>> v)
{
    for (auto& i : v)
    {
        for (auto& j : i)
            cout << j << " ";
        cout << "\n";
    }
}

int main()
{
    srand(time(0));
    //cout << "Matrix size: ";
    //cin >> 
    int c=n;
    vector<vector<double>> first(c, vector<double>(c, 0));
    fill(first);
    vector<vector<double>> second(c, vector<double>(c, 0));
    fill(second);
    // for(int i=4;i<=4;++i)
    // {
    //     cout<<"num of threads = "<<i<<"\n";
    //     omp_set_num_threads(i);
    //     double start = wtime();
    //     vector<vector<double>> res = matrix(first, second);
    //     double end = wtime();
    //     cout << fixed;
    //     cout.precision(10);
    //     cout << "size " << c << " - first  - "   << end - start << "\n";
    //     start = wtime();
    //     res = matrix2(first, second);
    //     end = wtime();
    //     cout << "size " << c << " - second - "  << end - start << "\n";
    // }
    //print(first);
    //print(second);
    for(int num_of_threads=6;num_of_threads<=10;++num_of_threads)
    {
        cout<<num_of_threads<<"\n";
        omp_set_num_threads(num_of_threads);
        double start = wtime();
        vector<vector<double>> res = matrix2(first, second);
        double end = wtime();
        cout << fixed;
        cout.precision(10);
        cout << "\t|openPM|  size = " << c << " (time = "   << end - start << ") \n";
        cout << "\t|PThread| size = " << c << " (time = " << doing_parallel(first, second, num_of_threads) <<")\n";
    }
    

    
    
    //print(result2);
    
    // omp_set_num_threads(4);
    // int i=0;
    // int counter = 0;
    // #pragma omp parallel for collapse(1)
    //      for(i=0;i<12;++i)
    //      {
    //         for(int j=0;j<12;++j)
    //         {
    //             cout<<" "<<omp_get_thread_num()<<" ";
    //            // #pragma omp atomic
    //             counter++;
    //         }
            
    //      }
            
    // cout<<"\n"<<counter;        
    // #pragma omp parallel
    // {
    //     #pragma omp critical
    //     {
    //         cout<<" ("<<omp_get_thread_num()<<") ";
    //     }
        
    // }
    
    
}
