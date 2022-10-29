#include "../../src/retrograde_analysis/cluster_support.hpp"
#include <cstddef>
#include <iostream>

struct null_type {};

template <typename NonPlacementDataType> 
void initialize_nonplacement_struct()
{
  int count = 0;
  int blocklengths[1];

  MPI_Aint displacements[1];
  
  MPI_Datatype types[1];
  
  MPI_Datatype tmp;
  MPI_Aint lowerBound;
  MPI_Aint extent;

  MPI_Type_create_struct(count, blocklengths, displacements, types,
      &tmp);
  MPI_Type_get_extent(tmp, &lowerBound, &extent);
  MPI_Type_create_resized(tmp, lowerBound, extent, &MPI_NonPlacementDataType);
  MPI_Type_commit(&MPI_NonPlacementDataType);
}

int main()
{
  MPI_Init(nullptr, nullptr);
  initialize_nonplacement_struct<null_type>();
  initialize_comm_structs<64, null_type>();

  int globalId = 0;
  int globalRank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &globalId);
  NodeCommData<64, null_type> n;
  MPI_Request r;

  if (globalId == 0)
  {
    n = { false, 3, 2, 1 };
    n.b.m_board[6] = 'K';

    MPI_Isend(&n, 1, MPI_NodeCommData, 1, 0, MPI_COMM_WORLD, &r);
  }

  else 
  {
    MPI_Irecv(&n, 1, MPI_NodeCommData, 0, 0, MPI_COMM_WORLD, &r);
  }

  MPI_Status s;
  MPI_Wait(&r, &s);

  if (globalId == 1)
  {
    std::cout << n.isWin << " " << n.depthToEnd << " " << n.remainingValidMoves << " " << n.G << std::endl;
    std::cout << "Board at set position: " << n.b.m_board[6] << std::endl;
  }
    

  MPI_Finalize();
}
