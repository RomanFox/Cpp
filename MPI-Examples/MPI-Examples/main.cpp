/*
* This example shows how to distribute workload to processors using MPI.
* When a processor terminates, the master sends a new chunk of data
* until the total input size has been distributed. After all data is
* distributed, a special value is sent indicating that the processor
* can terminate. The master tracks the number of active processors,
* and when all have finished, the master terminates too.
*/

#include <mpi.h>
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>
using std::ofstream;

#include <vector>
using std::vector;

#include <string>
#include <sstream>
#include <random>
#include <Windows.h>


int func_cheap(const int x) {
	return x + 1;
}

int func_expensive(const int x) {
	Sleep(100 * x);
	return func_cheap(x);
}

/* Input data generator, called by master code. */
vector<int> inputDataGenerator(const int N) {
	std::uniform_int_distribution<int> distribution(0, 10);
	std::default_random_engine generator;

	vector<int> data(N);
	for (int k = 0; k < N; k++) {
		int number = distribution(generator);
		data[k] = number;
	}
	return data;
}

/* Master code to be executed on rank 0. Sends data values to processors
* and listens for their output values. When a processor returns a value,
* a new value is sent if available. Otherwise the processor shall terminate.
* Finally, it is checked for errors.
*/
void masterCode(const int N, const int size) {

	int counter = 0;
	int errValue = -1;
	MPI_Request request;
	MPI_Status status;

	vector<int> input = inputDataGenerator(N); // input data to be distributed to processors
	vector<int> output(input.size()); // Output vector
	vector<int> inputIdx2proc(N);     // remember the processor number that computes the value for input at index m
	vector<int> proc2inputIdx(size);  // processor k computes input value index temp[k]

	int y = 0; // output value from processor 

	int kmax = min(size - 1, N); // number of input elements to be computed at start-up
	for (int k = 1; k < size; k++) {
		if (counter < kmax) {
			MPI_Isend(input.data() + counter, 1, MPI_INT, k, 0, MPI_COMM_WORLD, &request);
			inputIdx2proc[counter] = k;
			proc2inputIdx[k] = counter;
			counter++;
		}
		else { // if we have less input values than processors, send them an invalid input value to terminate
			MPI_Isend(&errValue, 1, MPI_INT, k, 0, MPI_COMM_WORLD, &request);
			proc2inputIdx[k] = -1; // processor terminated
		}
	}
	int N_activeProc = counter; // Number of active processors

								// While there are still processors running listen for any incoming messages
	while (N_activeProc > 0) {
		MPI_Recv(&y, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		int procNr = status.MPI_SOURCE;
		int idx = proc2inputIdx[procNr]; // input index that has been computed by processor k
		output[idx] = y;
		cout << "Rank 0: Obtain value from Processor " << procNr << endl;

		if (counter < input.size()) { // some more data to be computed, send another input value to this processor
			MPI_Isend(input.data() + counter, 1, MPI_INT, procNr, 0, MPI_COMM_WORLD, &request);
			inputIdx2proc[counter] = procNr;
			proc2inputIdx[procNr] = counter;
			counter++;
		}
		else { // no more data to be computed, send special value to terminate this processor
			MPI_Isend(&errValue, 1, MPI_INT, procNr, 0, MPI_COMM_WORLD, &request);
			proc2inputIdx[procNr] = -1;
			N_activeProc--;
		}
	}

	// Write output values to file for error checking
	ofstream file("rank-0.dat");
	for (int k = 0; k < N; k++) {
		file << input[k] << ", " << inputIdx2proc[k] << endl;
	}

	// Check for error by comparison of serial and parallel computation
	for (int k = 0; k < N; k++) {
		int err = func_cheap(input[k]) - output[k];
		if (err != 0) {
			cout << "Error at index " << k << ": " << func_cheap(input[k]) << ", " << output[k] << endl;
		}
	}
}

/* Code for slave processor. Master sends an input value to this processor:
* if it is valid, the output is computed and sent back to the master and
* waiting for another value. If the input value is invalid, the processor
* terminates.
*/
void slaveCode(const int rank) {
	cout << "Rank " << rank << " started " << endl;

	// Set output filename
	std::stringstream ss;
	ss << "rank-" << rank << ".dat";
	std::string filename = ss.str();
	ofstream file(filename);

	int x = 0; // input value sent from master to this slave
	int y = 0; // output value sent from this slave to master
	MPI_Status status;

	while (true) {
		MPI_Recv(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		if (x < 0) { // check input value
			break; // stop this loop 
		}
		y = func_expensive(x);
		file << x << ", " << rank << endl;
		MPI_Send(&y, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	cout << "Rank " << rank << " finished " << endl;
}

/* Main function. Start masterCode on rank 0 and slaveCode on ranks 1..N-1. */
int main(int argc, char** argv) {
	int mpiRank, mpiSize;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);

	if (mpiRank == 0) {
		cout << "Rank " << mpiRank << ": " << "MPI size = " << mpiSize << endl;
	}
	MPI_Barrier(MPI_COMM_WORLD);

	int N = 21; // input data size

	if (mpiSize < 2) {
		cout << "MPI size must be >= 2" << endl;
	}
	else {
		if (mpiRank == 0) {
			masterCode(N, mpiSize); // Master
		}
		else {
			slaveCode(mpiRank);  // Slaves
		}
	}

	cout << "Rank " << mpiRank << ": " << "Finished" << endl;
	MPI_Finalize();
	return 0;
}