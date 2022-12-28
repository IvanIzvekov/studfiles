#pragma warning (disable : 4996)
#include <iostream>
#include <string>
#include <time.h>
#include <cmath>

using namespace std;

int Conversion(string blockSize)
{
	long int deminishionByte;
	string valueDeminishion;
	if (blockSize.find("Kb") != -1)
	{
		deminishionByte = stol(blockSize.erase(blockSize.find("Kb")));
		deminishionByte = deminishionByte * 1024;
	}
	else if (blockSize.find("Mb") != -1)
	{
		deminishionByte = stol(blockSize.erase(blockSize.find("Mb")));
		deminishionByte = deminishionByte * 1024 * 1024;
	}
	else
	{
		deminishionByte = stol(blockSize);
	}
	return deminishionByte;
}

double TestRam(int quanityElements, string Type)
{
	double time = 0;
	int bufferElement;
	clock_t beginTime;
	clock_t endTime;
	int* testArray = new int[quanityElements];

	for (int i = 0; i < quanityElements; i++)
	{
		if (Type == "Write")
		{
			bufferElement = rand() % 100;

			beginTime = clock();
			testArray[i] = bufferElement;
			endTime = clock();
		}
		else
		{
			bufferElement = rand() % 100;
			testArray[i] = bufferElement;

			beginTime = clock();
			testArray[i];
			endTime = clock();
		}
		time += (double)(endTime - beginTime);
		/// CLOCKS_PER_SEC;
	}

	return time;
}

double TestMemory(int quanityElements, string Type, char* fileName)
{
	double time = 0;
	int* bufferArray = new int[quanityElements];
	int element;
	clock_t beginTime;
	clock_t endTime;
	FILE* file;

	if (Type == "Write") file = fopen(fileName, "w");
	else  file = fopen(fileName, "r");

	if (Type == "Write")
	{
		for (int i = 0; i < quanityElements; i++)
		{
			bufferArray[i] = rand() % 100;
			beginTime = clock();
			fprintf(file, "%d", bufferArray[i]);
			endTime = clock();

			time += (double)(endTime - beginTime);
		}
	}
	else
	{
		beginTime = clock();
		while ((element = fgetc(file)) != EOF)
		{
		};
		endTime = clock();

		time = (double)(endTime - beginTime);
	}
	fclose(file);
	return time;
}

void Test(int quanityElements, int launchCount, string memoryType, int launchNumber)
{
	double averageWrite = 0;
	double absErrorWrite = 0;
	double relErrorWrite = 0;
	double bandwidthWrite = 0;
	double dispersionWrite = 0;

	double averageRead = 0;
	double absErrorRead = 0;
	double relErrorRead = 0;
	double bandwidthRead = 0;
	double dispersionRead = 0;

	double readTime = 0;
	double writeTime = 0;

	double* testArrayRead = new double[quanityElements];
	double* testArrayWrite = new double[quanityElements];

	if (memoryType == "RAM")
	{
		writeTime = TestRam(quanityElements, "Write");
		readTime = TestRam(quanityElements, "Read");
		for (int i = 0; i < launchCount; i++)
		{
			testArrayWrite[i] = TestRam(quanityElements, "Write");
			testArrayRead[i] = TestRam(quanityElements, "Read");

			averageWrite += testArrayWrite[i];
			averageRead += testArrayRead[i];
		}
	}
	if (memoryType == "SSD")
	{
		char* fileName = (char*)"array.txt";

		writeTime = TestMemory(quanityElements, "Write", fileName);
		readTime = TestMemory(quanityElements, "Read", fileName);

		remove(fileName);

		for (int i = 0; i < launchCount; i++)
		{
			testArrayWrite[i] = TestMemory(quanityElements, "Write", fileName);
			testArrayRead[i] = TestMemory(quanityElements, "Read", fileName);

			averageWrite += testArrayWrite[i];
			averageRead += testArrayRead[i];
			remove(fileName);
		}
	}
	if (memoryType == "flash")
	{
		char* fileName = (char*)"D:\\array.txt";

		writeTime = TestMemory(quanityElements, "Write", fileName);
		readTime = TestMemory(quanityElements, "Read", fileName);

		remove(fileName);

		for (int i = 0; i < launchCount; i++)
		{
			testArrayWrite[i] = TestMemory(quanityElements, "Write", fileName);
			testArrayRead[i] = TestMemory(quanityElements, "Read", fileName);

			averageWrite += testArrayWrite[i];
			averageRead += testArrayRead[i];
			remove(fileName);
		}
	}

	averageWrite /= launchCount;
	averageRead /= launchCount;

	for (int i = 0; i < launchCount; i++)
	{
		dispersionWrite += pow(testArrayWrite[i] - averageWrite, 2);
		dispersionRead += pow(testArrayRead[i] - averageRead, 2);
	}

	absErrorWrite = sqrt(dispersionWrite / launchCount);
	relErrorWrite = (absErrorWrite / averageWrite) * 100;

	absErrorRead = sqrt(dispersionRead / launchCount);
	relErrorRead = (absErrorRead / averageRead) * 100;


	bandwidthWrite = (quanityElements * 4 / averageWrite) * pow(10, 6);
	bandwidthRead = (quanityElements * 4 / averageRead) * pow(10, 6);
	double memorySize = (double)(quanityElements * 4) / 1024 / 1024;
	delete[] testArrayRead;
	delete[] testArrayWrite;

	FILE* file;
	file = fopen("test.csv", "a");
	if (memoryType == "RAM") fprintf(file, "RAM;");
	if (memoryType == "SSD") fprintf(file, "SSD;");
	if (memoryType == "flash") fprintf(file, "flash;");
	fprintf(file, "%f;int;%d;clock();", memorySize, launchNumber);
	fprintf(file, "%f;%f;%f;%f;%f;", writeTime, averageWrite, bandwidthWrite / 1024 / 1024 / 8, absErrorWrite, relErrorWrite);
	fprintf(file, "%f;%f;%f;%f;%f;\n", readTime, averageRead, bandwidthRead / 1024 / 1024 / 8, absErrorRead, relErrorRead);
	fclose(file);
}

int main(int argc, char** argv)
{
	srand(time(NULL));

	//int launchCount = stoi(argv[6]);
	//const char* arg4 = argv[4];
	//const char* arg2 = argv[2];

	//string memoryType(arg2);

	//string blockSize(arg4);
	double mb4 = 524288;
	int launchCount = 10;
	string memoryType = "flash";
	string blockSize = "1Mb";
	int L1 = 589824;
	int L2 = 7864320;
	int L3 = 12582912;
	double deminishionByte = Conversion(blockSize);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 1);
	//Test(L2 / 4, launchCount, memoryType, 2);
	//Test(L3 / 4, launchCount, memoryType, 3);
	int quanityElements = deminishionByte / 4;
	for (int i = 0; i < 20; i++)
	{
		cout << i << "\n";
		int quanityElements = mb4 / 4;
		Test(quanityElements, launchCount, memoryType, i);
		mb4 += 524288;
	}
	return 0;
}