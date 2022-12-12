/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#include "../../src/retrograde_analysis/multi_node_impl.hpp"
#include <cstddef>
#include <iostream>

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
    n = { false, 2 };
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
    std::cout << n.winLabel << " " << n.G << std::endl;
    std::cout << "Board at set position: " << n.b.m_board[6] << std::endl;
  }
    

  MPI_Finalize();
}
