# Scrappy Tablebase Generator for Chess Variants

The config.json file will need to be changed to account for whatever game you wish to run. The following is a description of each item:

```
ROW_SZ = number of rows for the board

COL_SZ = number of columns for the board

N_MAN = number of pieces you wish to include in your tablebase

SRC_DIRS = all paths containing files needed to operate your game

INCLUDE_HEADERS = paths containing header files

NO_ROYALTY_PIECESET = list of notation of all non-royalty pieces in your game

ROYALTY_PIECESET = list of all royalty pieces in your game

FORWARD_MOVE_GENERATOR = name of your functor that produces forward moves for pieces

REVERSE_MOVE_GENERATOR = name of your functor that produces reverse moves for pieces

WIN_COND_EVALUATOR = name of your functor that checks if a state is a checkmate state

BOARD_PRINTER = name of your functor that prints boardstates

NON_PLACEMENT_DATATYPE = domain specific data that is stored within a board state

IS_VALID_BOARD_FN = name of your functor that checks if a boardstate is legal

HZ_SYM_EVALUATOR = condition for symmetry across horizontal board axis

VT_SYM_EVALUATOR = condition for symmetry across vertical board axis
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

The above example represents a white queen, black king, and white king in standard chess.
