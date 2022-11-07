//
// MPI Solution for asynchronous sending / receiving of an unknown number of elements.
// the asynchronous send delivers the message to the recipient and the receive writes it to
// memory.
//
// MPI tags are utilized to denote that the final message within and iteration is being sent. After this point
// in the retrograde analysis implementation, an MPI barrier will be invoked. 
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

  int n = 0;
  int rank = 0;
  
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // let 0 be the receiver
  if (rank == RECEIVER)
  {
    // Asynchronous barrier by the receiver. Reached before any work is done
    // by the senders
    MPI_Request request;
    int index = 0;
    int count = 0;
    int reduced_end = 1; // starts at one to ignore the receiving process.
    do
    {
      MPI_Status status;
      MPI_Irecv(&buffer[0], BUF_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, 
          MPI_COMM_WORLD, &request);

      MPI_Wait(&request, &status);
      reduced_end += status.MPI_TAG;
      ++count;
    } while (reduced_end != n); // we need to receive end flags from n-1 processes
    
    // ensure that all are received 
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
      if (i != num_requests - 1) // standard request. still more data to process
      {
        MPI_Isend(&buffer[0], BUF_SIZE, MPI_INT, RECEIVER, 0, MPI_COMM_WORLD,
            &request);
      }
      else // if we know it is the last request for the iteration
      {
        MPI_Isend(&buffer[0], BUF_SIZE, MPI_INT, RECEIVER, 1, MPI_COMM_WORLD,
            &request);
      }
      MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
  }

  MPI_Finalize();
  return 0;
}
