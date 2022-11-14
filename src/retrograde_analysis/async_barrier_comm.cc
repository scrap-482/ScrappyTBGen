//
// MPI asynchronous solution to communication scheme.
// This seems to be unsafe. MPI Sends are guaranteed to be 
// received in the order of sends (from the same process) but this guarantee
// is not true for asynchronous barriers (despite being sent after the last asynchronous send,
// the barrier is sometimes delivered first and messages are lost
//
#include <mpi.h>
#include <iostream>
#include <vector>
#include <array>
#include <unistd.h>

#define BUF_SIZE 8 
#define RECEIVER 0

int main()
{
  MPI_Init(NULL, NULL);
   
  std::array<int, BUF_SIZE> buffer;
  std::array<MPI_Request, 2> requests;

  int n;
  int rank;
  
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // let 0 be the receiver
  if (rank == RECEIVER)
  {
    // Asynchronous barrier by the receiver. Reached before any work is done
    // by the senders
    MPI_Ibarrier(MPI_COMM_WORLD, &requests[0]);
    int index = 0;
    int count = 0;
    do
    {
      MPI_Irecv(&buffer[0], BUF_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, 
          MPI_COMM_WORLD, &requests[1]);

      MPI_Waitany(2, &requests[0], &index, MPI_STATUSES_IGNORE); 
      ++count;
    } while (index == 1); // before barriers are set
    
    // cancel our initial request
    MPI_Cancel(&requests[1]);
    std::cout << "number of receives " << count << std::endl;
  }
  else 
  {
    // send / recv 12 messages. however, the receiver does not know this number
    constexpr int num_requests = 12;
    MPI_Request request;

    for (int i = 0; i < buffer.size(); ++i)
      buffer[i] = rank;

    for (int i = 0; i < num_requests; ++i)
    {
      MPI_Isend(&buffer[0], BUF_SIZE, MPI_INT, RECEIVER, 0, MPI_COMM_WORLD,
          &request);
      MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
     
    MPI_Ibarrier(MPI_COMM_WORLD, &requests[1]);
    MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
  }

  MPI_Finalize();
  return 0;
}
