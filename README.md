# Scrappy Tablebase Generator for Chess Variants

The Scrappy Tablebase Generator is an open-source project implemented to allow for easy generation of tablebases for chess variants. Our implementation
utilizes C++ templating to implement a generalized version of retrograde analysis, the backwards induction algorithm used to generate tablebases, in a rule agnostic manner. 
To implement specific rulesets of a game, our implementation invokes user-defined callbacks to generate moves, check move validity, and determine if a position is an 
end-of-game (or checkmate) state. Users designing custom variants are required to implement a list of functionalities as C++ functors
which are then specified in a configuration file and integrated into the SCons build process.

## Dependencies
Out of the box functionality is currently only ensured for Linux systems. The following dependencies are required to use our tablebase generator:
* GCC 11 (older versions with experimentatal C++20 implementations may work with the use2a=true flag in SCons)
* SCons v3 (older versions may also be compatible but no guarantees are made)
* If cluster support is required, then Open-MPI 4.1

## Example Variant Implementations
To get started with implementing variants of your choice, or if you are just interested in generating tablebases for games we already support,
`ScrappyTBGen/src/rules` contains two games that have been fully implemented. The first is simply the game of standard chess. We have generated this implementation 
located in `ScrappyTBGen/src/rules/chess` in 
order to validate the correctness of our retrograde analysis implementation and as an easy way to get started with our generator. The other game we current support
is the game of Capabalanca chess located in `ScrappyTBGen/src/rules/capablanca`. This variant is played on a 10&times;8 board and introduces the archbishop and 
chancellor pieces to the game.

## Generating Tablebases for a Custom Variant

To support tablebase generation for a custom variant, the user must inherit and implement the purely virtual functors provided in 
`ScrappyTBGen/src/retrograde_analysis/state_transition.hpp`. An example of one such functor to implement is the reverse
move mechanism showcased below:
```cpp
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateReverseMoves 
{
public:
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};
```
We describe the necessary functors to support a variant in the following table

| **Functor** | **Description** |
|-------------|-----------------|
|GenerateForwardMoves|Takes in a single board state and returns a vector of all moves from the current position|
|GenerateReverseMoves|Takes in a single board state and returns a vector of all moves that lead to the current position|
|CheckmateEvaluator|Determines if the given board state is an end-of-game or checkmate state|
|BoardPrinter|Displays the board in an unicode friendly format|
|ValidBoardEvaluator|Determines if the given board configuration is a valid state in the game|

Furthermore, the optional functors `HzSymEvaluator` and `VtSymEvaluator` exploit horizontal and vertical symmetry for certain piece combinations.
These may be optionally passed but more work is required to ensure that the retrograde analysis algorithm fully exploits these symmetries. Within the
`ScrappyTBGen/src/core` folder, we provide several class hierarchies which generalize many of the ruleset features across chess variants. Usage of these features
may be useful to streamline the development process of variant rulesets. Examples utilizing these features are demonstrated in `ScrappyTBGen/src/rules/chess`
and `ScrappyTBGen/src/rules/capablanca`.

## JSON Configuration File

Once the ruleset for a variant has been fully implemented, it is necessary to create a configuration file in order for the SCons build system
to build the correct source files, include the correct headers, and use the correct type names for the user defined functors. The config file specification
is what bridges the gap between the user-defined ruleset implementation and the internal retrograde analysis code. The file may be placed wherever you wished, but
the path must be passed to SCons as a command line argument. The following table demonstrates all of the currently supported properties in the config file.

|**Key**|**Value**|
|-------|---------|
|ROW_SZ|number of rows for the board|
|COL_SZ|number of columns for the board|
|N_MAN|number of pieces you wish to include in your tablebase|
|SRC_DIRS|all paths containing files needed to operate your game|
|INCLUDE_HEADERS|paths containing header files|
|NO_ROYALTY_PIECESET|list of notation of all non-royalty pieces in your game|
|ROYALTY_PIECESET|list of all royalty pieces in your game|
|FORWARD_MOVE_GENERATOR|name of your functor that produces forward moves for pieces|
|REVERSE_MOVE_GENERATOR|name of your functor that produces reverse moves for pieces|
|WIN_COND_EVALUATOR|name of your functor that checks if a state is a checkmate state|
|BOARD_PRINTER|name of your functor that prints board states|
|NON_PLACEMENT_DATATYPE|domain specific data that is stored within a board state|
|IS_VALID_BOARD_FN|name of your functor that checks if a boardstate is legal|
|HZ_SYM_EVALUATOR|condition for symmetry across horizontal board axis|
|VT_SYM_EVALUATOR|condition for symmetry across vertical board axis|

An example configuration for the example Capablanca implementation is displayed below:
```json
{
  "ROW_SZ"   : 10,
  "COL_SZ"   : 8,
  "N_MAN"    : 3,
  "SRC_DIRS" : [
    "src/rules/capablanca",
    "src/core",
    "src/utils"
  ],
  "INCLUDE_HEADERS" : [
    "src/rules/capablanca/interface.h"
  ],
  "NO_ROYALTY_PIECESET"     : ["p", "P", "r", "R", "n", "N", "b", "B", "q", "Q", "a", "A", "c", "C"],
  "ROYALTY_PIECESET"        : ["k", "K"],
  "FORWARD_MOVE_GENERATOR"  : "CapablancaGenerateForwardMoves",
  "REVERSE_MOVE_GENERATOR"  : "CapablancaGenerateReverseMoves",
  "WIN_COND_EVALUATOR"      : "CapablancaCheckmateEvaluator",
  "BOARD_PRINTER"           : "CapablancaBoardPrinter",
  "NON_PLACEMENT_DATATYPE"  : "CapablancaNPD",
  "IS_VALID_BOARD_FN"       : "CapablancaValidBoardEvaluator",
  "HZ_SYM_EVALUATOR"        : null,
  "VT_SYM_EVALUATOR"        : null
}
```
## Parallel Implementation
Two implementations of retrograde analysis are implemented: an implementation for shared-memory systems using OpenMP and an implementation for 
distributed memory implementation using MPI. The OpenMP implementation parallelizes the exploration of predecessor states at each iteration in the retrograde
analysis implementation. The MPI implementation utilizes a nonblocking point-to-point communication scheme based upon the work in the following paper: 
`Makhnychev Vladimir Sergeevich, “Parallelization of retroanalysis algorithms for solving enumeration problems in computing systems without shared memory,” Moscow 
State University, Russia, 2012.`

Command line flags in the build system dictate which algorithm is invoked. If using the shared memory implementation (default in the build system), then OpenMP will select
a desirable number of threads to use on your system at runtime. If this is not wanted, you can limit the number of threads OpenMP uses with the `OMP_NUM_THREADS` 
environment variable. To configure the number of MPI processes to be utilized, you must do this when executing the binary with `mpirun`.

## Compilation Instructions
To compile, run:
```
scons --config_dir=<path/to/config.json> [--enable_cluster] [use2a=true]
```

For example, 
```
scons --config_dir=src/rules/chess/config.json use2a=true
```

Afterwards, a folder will be produced called "compiled". Within that folder will be an executable called "scrappytbgen". Invoke that executable
followed by the pieces you wish to use to produce the tablebase. Do not separate the pieces by commas or spaces.

For example,
### Single Node System
```
./scrappytbgen QkK
```
### Multi Node System (with Open-MPI)
```
mpirun -np <number_of_processes> ./compiled/scrappytbgen
```

The above example on both architectures represents a white queen, black king, and white king in standard chess. The following convention of utilizing uppercase letters 
for white the white piece set and lowercase letters for the black piece set is utilized in current ruleset implementations. After executing the binary, the executable
will run. After the tablebase is generated, data about the collected results will be output, and you will be prompted to input board states in the command line
terminal to determine their depth-to-mate and display optimal game playout. Further work needs to be completed in order to store the tablebases on a hard drive.

## Contributing
This is an open-source project, and we greatly support any community-driven contributions. To contribute, initiate a pull request with an explanation of the 
implemented feature or bugfix.
