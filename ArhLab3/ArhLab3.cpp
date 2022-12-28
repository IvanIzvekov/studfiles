#include <stdlib.h>
#include <math.h>
#include <time.h>
//#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>

const int m = 1000;
const int n = 1000;
double lNum = 10.0;

using namespace std;

template <typename T>
void Multiplication(vector<vector<T>>& matrix, vector<T>& v, vector<T>& vectorIt)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            vectorIt.push_back(matrix[i][j] * v[j]);
    }
}

/*double wtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}*/

template <typename T>
double test()
{
    srand(time(0));
    vector<vector<T>> matrix(m, vector<T>(0));
    vector<T> v;
    vector<T> ansver;
    double t = 0;

    for (int i = 0; i < m; i++)
    {

        for (int j = 0; j < n; j++)
            matrix[i].push_back(rand() % 1000);

    }
    for (int j = 0; j < n; j++)
        v.push_back(rand() % 1000);


    t = static_cast<double>(clock()) / CLOCKS_PER_SEC;
    Multiplication<T>(matrix, v, ansver);
    t = static_cast<double>(clock()) / CLOCKS_PER_SEC - t;

    return t;
}


template <typename T>
void Math()
{
    double dispersion = 0, average = 0, deviation = 0, taskPerf = 0, absError = 0, relError = 0;
    int instructionCount = n * (m + (n - 1)) + m;
    vector<double> testAr(10);
    double t = test<T>();

    //for (int i = 0; i < 10; i++)
    //{
     //   testAr[i] += test<T>();
      //  average += testAr[i];
    //}

    for (int i = 0; i < 10; i++)
    {
        testAr[i] = test<T>();
        average += testAr[i];
        taskPerf += (instructionCount / testAr[i]);
        dispersion += pow(testAr[i] - average, 2);
    }

    taskPerf = pow((1 / taskPerf), -1);
    deviation = sqrt(dispersion / lNum);
    average /= lNum;//среднеквадратичное отклонение
    absError = abs(average - t);
    relError = (absError / average) * 100;
 cerr << "error";
    ofstream file("file.cvs", ios::app);
    file << "\nInstruction Count: " << instructionCount
        << "\nTimer: time()"
        << "\nTime: " << t
        << "\nnLNum: " << lNum
        << "\nAvTime: " << average
        << "\nAbsErr: " << absError
        << "\nRelErr: " << relError
        << "\nTaskPer: " << taskPerf;

    file.close();
    t = 0;
   
    /*
    cout<<"\nInstruction Count: " << instructionCount
    << "\nTimer: time()"
    << "\nTime: " << t
    << "\nnLNum: " << lNum
    << "\nAvTime: " << average
    << "\nAbsErr: " << absError
    << "\nRelErr: " << relError
    << "\nTaskPer: " << taskPerf;*/
}

int main()
{
    ofstream file2("file.cvs", ios::out);
    file2 << "\nTask: multiplying a matrix by a vector OpType: double";
    file2.close();
    Math<double>();
    ofstream file3("file.cvs", ios::app);
    file3 << "\n\nTask: multiplying a matrix by a vector OpType: float";
    file3.close();
    Math<float>();
    ofstream file4("file.cvs", ios::app);
    file4 << "\n\nTask: multiplying a matrix by a vector OpType: int";
    file4.close();
    Math<int>();
    ofstream file5("file.cvs", ios::app);
    file5 << "\n\nTask: multiplying a matrix by a vector OpType: long long";
    file5.close();
    Math<long long>();
    return 0;
}