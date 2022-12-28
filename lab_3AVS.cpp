#include <iostream>
#include <string>
#include <vector>
//#include <time.h>
#include <sys/time.h>
#include <cmath>
#include <fstream>

using namespace std;

long double wtime();

long double WriteRam(int size) // размер в байтах
{
	size /= 4; //сколько нужно int значений, чтобы получить необходимый размер
	int* data = new int[size];

	long double allTime = 0;
	
	for (int i = 0; i < size; ++i)
	{
		int r = rand();
		long double start = wtime();
		data[i] = r;
		long double end = wtime();
		allTime += end - start;
	}
		
	delete data;
	return allTime;
}

long double ReadRam(int size)
{
	size /= 4;
	int* data = new int[size];
	for (int i = 0; i < size; ++i)
		data[i] = rand();

	long double start = wtime();
	for (int i = 0; i < size; ++i)
		data[i];
	long double end = wtime();
	delete data;
	return end - start;
}

long double WriteSSD(int size) // размер в байтах
{
	//size/=4; //сколько нужно int значений, чтобы получить необходимый размер
	remove("megaBenchmark.txt");
	ofstream file("megaBenchmark.txt");
	long double start = wtime();
	for (int i = 0; i < size; ++i)
		file << int(rand() % 10);
	long double end = wtime();
	file.close();
	return end - start;
}

long double ReadSSD(int size) // размер в байтах
{
	//size/=4; //сколько нужно int значений, чтобы получить необходимый размер
	WriteSSD(size);
	char a;
	ifstream file("megaBenchmark.txt");
	long double start = wtime();
	for (int i = 0; i < size; ++i)
		file;
	long double end = wtime();
	return end - start;
}

long double WriteFlash(int size) // размер в байтах
{
	//size/=4; //сколько нужно int значений, чтобы получить необходимый размер
	remove("../../../../d/megaBenchmark.txt");
	ofstream file("../../../../d/megaBenchmark.txt");
	long double start = wtime();
	for (int i = 0; i < size; ++i)
		file << int(rand() % 10);
	long double end = wtime();
	file.close();
	return end - start;
}

long double ReadFlash(int size) // размер в байтах
{
	//size/=4; //сколько нужно int значений, чтобы получить необходимый размер
	WriteFlash(size);
	//char a;
	ifstream file("../../../../d/megaBenchmark.txt");
	long double start = wtime();
	for (int i = 0; i < size; ++i)
		file;
	long double end = wtime();
	return end - start;
}


string memoryType = "";
string memorySize = "";
int blockSize;
string elementType = "int";
int bufferSize = 2147483647;
int launchNum = 0;

int main(int argl, char* argv[])
{
	srand(time(0));
	setlocale(0, "");
	//cout<<"argl = " << argl << "\n";  
	for (int i = 1; i < argl; ++i)
	{
		//cout<<"argv["<<i<<"] = '"<<argv[i] << "'\n";
		if (argv[i] == "-m")
		{
			//cout<<"1111";
			memoryType = argv[i + 1];
		}

		else if (argv[i] == "-b")
		{
			memorySize = argv[i + 1];
			if (memorySize.find("kb") != string::npos)
				blockSize = stoi(memorySize.replace(memorySize.find("kb"), 2, "")) * 1024;
			else if (memorySize.find("mb") != string::npos)
				blockSize = stoi(memorySize.replace(memorySize.find("kb"), 2, "")) * 1024 * 1024;
			else
				blockSize = stoi(memorySize);
		}
		else if (argv[i] == "-l")
			launchNum = stoi(argv[i + 1]);
	}
	//cout<<"-m = " << memoryType;
	//if (memoryType == "")
		memoryType = "RAM";
	//if (memorySize == "")
		blockSize = 1024;
	//if (launchNum == 0)
		launchNum = 10;
	ofstream file("out.csv");
	file << "Num;mType;bSize;eType;bSize;lNum;fun;wTimes[i];avarageWTime;writeBandwidth;wAbs[i];wRel[i];rTimes[i];avarageRTime;readBandwidth;rAbs[i];rRel[i];\n";

	file << fixed;
	file.precision(10);
	vector<long double> L = { 0.387 * 1024 * 1024, 3 * 1024 * 1024,  8 * 1024 * 1024, 12 * 1024 * 1024};

	ofstream file2("out2.csv");
	file2 << "Num;mType;bSize;eType;bSize;lNum;fun;avarageWTime;writeBandwidth;avarageRTime;readBandwidth;\n";

	file2 << fixed;
	file2.precision(10);

	for (int j = 0; j < 4; ++j)
	{
		blockSize = L[j];
		cout << "L" << j + 1 << " : BlockSize = " << blockSize << "\n";
		/*for (int i = 0; i < 3; ++i)
			cout << "memoryType = " << memoryType << "\nBlockSize = " << blockSize << "\nLaunchNum = " << launchNum << "\n";*/
		vector<long double> readTimes;
		vector<long double> writeTimes;
		cout << fixed;
		cout.precision(8);
		long double sumReadTime = 0;
		long double sumWriteTime = 0;

		for (int curentNum = 1; curentNum <= launchNum; ++curentNum)
		{
			if (memoryType == "RAM")
			{
				readTimes.push_back(ReadRam(blockSize));
				writeTimes.push_back(WriteRam(blockSize));
			}
			else if (memoryType == "SSD")
			{
				readTimes.push_back(ReadSSD(blockSize));
				writeTimes.push_back(WriteSSD(blockSize));
			}

			sumWriteTime += writeTimes.back();
			sumReadTime += readTimes.back();
		}
		long double avarageWriteTime = sumWriteTime / launchNum;
		long double avarageReadTime = sumReadTime / launchNum;
		long double writeBandwidth = (blockSize / avarageWriteTime) * 100000;
		long double readBandwidth = (blockSize / avarageReadTime) * 1000000;
		vector<long double> readAbsError;
		vector<long double> readRelError;
		vector<long double> writeAbsError;
		vector<long double> writeRelError;
		for (int i = 0; i < launchNum; ++i)
		{
			readAbsError.push_back(abs(readTimes[i] - avarageReadTime));
			readRelError.push_back((readAbsError[i] / readTimes[i]) * 100);

			writeAbsError.push_back(abs(writeTimes[i] - avarageWriteTime));
			writeRelError.push_back((writeAbsError[i] / writeTimes[i]) * 100);
			//cout<<"CurentNum = "<<i+1<<"; readTime = " << readTimes[i] << "; writeTime = " << writeTimes[i] << "; readAbs = " << readAbsError[i] << "; readRel = " 
			//<< readRelError[i] << "; writeAbs  = " << writeAbsError[i] << "; writeRel = " << writeRelError[i] << "\n";
		}

		
		for (int i = 0; i < launchNum; ++i)
			file << i + 1 << ";" << memoryType << ";" << blockSize << ";" << elementType << ";" << bufferSize << ";" << launchNum << ";" << "gettimeofday" << ";" << writeTimes[i] << ";" << avarageWriteTime << ";"
			<< writeBandwidth << ";" << writeAbsError[i] << ";" << writeRelError[i] << ";" << readTimes[i] << ";" << avarageReadTime << ";" << readBandwidth << ";" << readAbsError[i] << ";" << readRelError[i] << ";\n";
		file2 << j + 1 << ";" << memoryType << ";" << blockSize << ";" << elementType << ";" << bufferSize << ";" << launchNum << ";" << "gettimeofday" << ";" << avarageWriteTime << ";"
			<< writeBandwidth << ";" << avarageReadTime << ";" << readBandwidth << ";\n";

	}
	file.close();
	file2.close();

	//if (memoryType == "")
	memoryType = "SSD";
	//if (memorySize == "")
	blockSize = 0;
	//if (launchNum == 0)
	launchNum = 10;

	ofstream file3("outSSD.csv");
	file3 << "Num;mType;bSize;eType;bSize;lNum;fun;wTimes[i];avarageWTime;writeBandwidth;wAbs[i];wRel[i];rTimes[i];avarageRTime;readBandwidth;rAbs[i];rRel[i];\n";

	file3 << fixed;
	file3.precision(10);
	//vector<long double> L = { 64, 0.187 * 1024 * 1024, 3 * 1024 * 1024,  8 * 1024 * 1024, 12 * 1024 * 1024 };

	ofstream file4("outSSD2.csv");
	file4 << "mType;bSize;eType;bSize;lNum;fun;avarageWTime;writeBandwidth;avarageRTime;readBandwidth;\n";

	file4 << fixed;
	file4.precision(10);

	for(int j=0;j<3;++j)
	{
		blockSize += 4 * 1024 * 1024;
		cout << "SSD" << " : BlockSize = " << blockSize << "\n";
		/*for (int i = 0; i < 3; ++i)
			cout << "memoryType = " << memoryType << "\nBlockSize = " << blockSize << "\nLaunchNum = " << launchNum << "\n";*/
		vector<long double> readTimes;
		vector<long double> writeTimes;
		cout << fixed;
		cout.precision(8);
		long double sumReadTime = 0;
		long double sumWriteTime = 0;
		vector<int> bSizes;
		
		
		for (int curentNum = 1; curentNum <= launchNum; ++curentNum)
		{
			
			bSizes.push_back(blockSize);
			if (memoryType == "RAM")
			{
				readTimes.push_back(ReadRam(blockSize));
				writeTimes.push_back(WriteRam(blockSize));
			}
			else if (memoryType == "SSD")
			{
				readTimes.push_back(ReadSSD(blockSize));
				writeTimes.push_back(WriteSSD(blockSize));
			}

			sumWriteTime += writeTimes.back();
			sumReadTime += readTimes.back();
		}
		long double avarageWriteTime = sumWriteTime / launchNum;
		long double avarageReadTime = sumReadTime / launchNum;
		long double writeBandwidth = (blockSize / avarageWriteTime) / 100000;
		long double readBandwidth = (blockSize / avarageReadTime) / 1000000;
		vector<long double> readAbsError;
		vector<long double> readRelError;
		vector<long double> writeAbsError;
		vector<long double> writeRelError;
		for (int i = 0; i < launchNum; ++i)
		{
			readAbsError.push_back(abs(readTimes[i] - avarageReadTime));
			readRelError.push_back((readAbsError[i] / readTimes[i]) * 100);

			writeAbsError.push_back(abs(writeTimes[i] - avarageWriteTime));
			writeRelError.push_back((writeAbsError[i] / writeTimes[i]) * 100);
			//cout<<"CurentNum = "<<i+1<<"; readTime = " << readTimes[i] << "; writeTime = " << writeTimes[i] << "; readAbs = " << readAbsError[i] << "; readRel = " 
			//<< readRelError[i] << "; writeAbs  = " << writeAbsError[i] << "; writeRel = " << writeRelError[i] << "\n";
		}


		for (int i = 0; i < launchNum; ++i)
			file3 << i + 1 << ";" << memoryType << ";" << bSizes[i] << ";" << elementType << ";" << bufferSize << ";" << launchNum << ";" << "gettimeofday" << ";" << writeTimes[i] << ";" << avarageWriteTime << ";"
			<< writeBandwidth << ";" << writeAbsError[i] << ";" << writeRelError[i] << ";" << readTimes[i] << ";" << avarageReadTime << ";" << readBandwidth << ";" << readAbsError[i] << ";" << readRelError[i] << ";\n";
		file4 << memoryType << ";" << blockSize << ";" << elementType << ";" << bufferSize << ";" << launchNum << ";" << "gettimeofday" << ";" << avarageWriteTime << ";"
			<< writeBandwidth << ";" << avarageReadTime << ";" << readBandwidth << ";\n";
	}
	file3.close();
	file4.close();

	// <<"\nWriteFlash" << WriteFlash(1024*1024) <<"\nReadFlash: " << ReadFlash(1024*1024)<<"\n";
}









long double wtime()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	//std::cerr<<(long double)t.tv_sec <<"+"<< (long double)t.tv_usec * 1E-6<<"="<<(long double)t.tv_sec + (long double)t.tv_usec * 1E-6<<"\n";
	return (long double)t.tv_sec + (long double)t.tv_usec * 1E-6;
}
/*
int Conversion(std::string blockSize)
{
	long int deminishionByte;
	std::string valueDeminishion;
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

long double TestRam(int quanityElements, std::string Type)
{
	long double time = 0;
	int bufferElement;
	clock_t beginTime;
	clock_t endTime;
	int* testArray = new int[quanityElements];

	for (int i = 0; i < quanityElements; i++)
	{
		if (Type == "Write")
		{

			beginTime = wtime();
			testArray[i] = rand() % 100;
			endTime = wtime();
		}
		else
		{
			bufferElement = rand() % 100;
			testArray[i] = bufferElement;

			beginTime = wtime();
			testArray[i];
			endTime = wtime();
		}
		time += (long double)(endTime - beginTime); // CLOCKS_PER_SEC;
	}

	return time;
}

long double TestMemory(int quanityElements, std::string Type, char* fileName)
{
	long double time = 0;
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
			beginTime = wtime();
			fprintf(file, "%d", bufferArray[i]);
			endTime = wtime();

			time += (long double)(endTime - beginTime);
		}
	}
	else
	{
		beginTime = wtime();
		while ((element = fgetc(file)) != EOF)
		{
		};
		endTime = wtime();

		time = static_cast<long double>(endTime - beginTime);
		std::cerr<<beginTime<<" "<<endTime<<" "<<time<<std::endl;
	}
	fclose(file);
	return time;
}

void Test(int quanityElements, int launchCount, std::string memoryType, int launchNumber)
{
	long double averageWrite    = 0;
	long double absErrorWrite   = 0;
	long double relErrorWrite   = 0;
	long double bandwidthWrite  = 0;
	long double dispersionWrite = 0;

	long double averageRead     = 0;
	long double absErrorRead    = 0;
	long double relErrorRead    = 0;
	long double bandwidthRead   = 0;
	long double dispersionRead  = 0;

	long double readTime        = 0;
	long double writeTime       = 0;

	long double* testArrayRead  = new long double[quanityElements];
	long double* testArrayWrite = new long double[quanityElements];

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
		char* fileName = (char *)"array.txt";

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


	bandwidthWrite = (quanityElements * 4 / averageWrite) * pow(10,6);
	bandwidthRead = (quanityElements * 4 / averageRead) * pow(10, 6);
	long double memorySize = (long double)(quanityElements * 4) / 1024 / 1024;
	delete[] testArrayRead;
	delete[] testArrayWrite;

	FILE* file;
	file = fopen("test.csv", "a");
	if (memoryType == "RAM") fprintf(file, "RAM;");
	if (memoryType == "SSD") fprintf(file, "SSD;");
	if (memoryType == "flash") fprintf(file, "flash;");
	fprintf(file, "%f;int;%d;wtime();", memorySize, launchNumber);
	fprintf(file, "%f;%f;%f;%f;%f;", writeTime, averageWrite, bandwidthWrite/1024/1024/8, absErrorWrite, relErrorWrite);
	fprintf(file, "%f;%f;%f;%f;%f;\n", readTime, averageRead, bandwidthRead/1024/1024/8, absErrorRead, relErrorRead);
	fclose(file);
}

int main(int argc, char** argv)
{
	srand(time(NULL));

	//int launchCount = std::stoi(argv[6]);
	//const char* arg4 = argv[4];
	//const char* arg2 = argv[2];

	//std::string memoryType(arg2);

	//std::string blockSize(arg4);
	long double mb4 = 524288;
	int launchCount = 10;
	std::string memoryType = "flash";
	std::string blockSize = "1Mb";
	int L1 = 589824;
	int L2 = 7864320;
	int L3 = 12582912;
	long double deminishionByte = Conversion(blockSize);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 0);
	//Test(L1 / 4, launchCount, memoryType, 1);
	//Test(L2 / 4, launchCount, memoryType, 2);
	//Test(L3 / 4, launchCount, memoryType, 3);
	int quanityElements = deminishionByte / 4;
	for (int i = 0; i < 10; i++)
	{
		std::cout << i << "\n";
		int quanityElements = mb4 / 4;
		Test(quanityElements, launchCount, memoryType, i);
		mb4 += 524288;
	}
	return 0;
}*/
