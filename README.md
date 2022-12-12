# Scrappy Tablebase Generator for Chess Variants

The Scrappy Tablebase Generator is an open-source project implemented to allow for easy generation of tablebases for chess variant games. Our implementation
works utilizing C++ templating to implement retrograde analysis, the backwards induction algorithm used to generate tablebases, in a rule agnostic manner. 
To implement specific rulesets of a game, we invoke user-defined callbacks to generate moves, check move validity, and determine if a position is an 
end-of-game (or checkmate) state. Users designing a variant to generate tablebases for must implement a list of functionalities as C++ functors
which are then specified in a configuration file and are integrated into the SCons build process.

## Example Variant Implementations
To get you started with supporting variants of your choice or if you are just in the interesting of generating tablebases for games we already support,
`ScrappyTBGen/src/rules` contains to games that have been fully implemented. The first is simply the game of standard chess. We have generated this implementation 
located in `ScrappyTBGen/src/rules/chess` in 
order to validate the correctness of our retrograde analysis implementation and as an easy way to get started with our generator. The other game we current support
is the game of Capabalanca chess located in `ScrappyTBGen/src/rules/capablanca`. This variant is played on a 10&times;8 board and introduces the archbishop and 
chancellor pieces to the game.

## Generating Tablebases for a Custom Variant

To support tablebase generation for a custom variant, the user must extend and implement the purely virtual functors provided in 
`ScrappyTBGen/src/retrograde_analysis/state_transition.hpp`. An example of one such functor to implement the reverse
move mechanism is showcased below:
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
`ScrappyTBGen/src/core`, we provide several class hierarchies which generalize many of the ruleset features across chess variants. Usage of these features
may be useful to streamline the development process of variant rulesets. Examples utilizing these features are demonstrated in `ScrappyTBGen/src/rules/chess`
and `ScrappyTBGen/src/rules/capablanca`.

## JSON Configuration File

The config.json file will need to be changed to account for whatever game you wish to run. The following is a description of each item:

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
|BOARD_PRINTER|name of your functor that prints boardstates|
|NON_PLACEMENT_DATATYPE|domain specific data that is stored within a board state|
|IS_VALID_BOARD_FN|name of your functor that checks if a boardstate is legal|
|HZ_SYM_EVALUATOR|condition for symmetry across horizontal board axis|
|VT_SYM_EVALUATOR|condition for symmetry across vertical board axis|

## Compilation Instructions
```

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
```
./scrappytbgen QkK
```

The above example represents a white queen, black king, and white king in standard chess. The following convention of utilizing uppercase letters for 
white the white piece set and lowercase letters for the black piece set is utilized in current rulset implementations.
