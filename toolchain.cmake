set(BRANCH
    "branch-8.8"
    CACHE STRING
          "Can be overridden during cmake configuration, -DBRANCH=branch-x.x")

set(CMAKE_C_COMPILER
    /prj/qct/llvm/release/internal/HEXAGON/${BRANCH}/linux64/latest/Tools/bin/hexagon-clang
    CACHE FILEPATH "Hexagon C compiler")
set(CMAKE_OBJCOPY
    /prj/qct/llvm/release/internal/HEXAGON/${BRANCH}/linux64/latest/Tools/bin/hexagon-llvm-objcopy
    CACHE FILEPATH "Hexagon LLLVM objcopy")
set(HEXAGON_SIMULATOR
    /prj/qct/llvm/release/internal/HEXAGON/${BRANCH}/linux64/latest/Tools/bin/hexagon-sim
    CACHE FILEPATH "Hexagon Simulator")
set(CMAKE_RANLIB
    /prj/qct/llvm/release/internal/HEXAGON/${BRANCH}/linux64/latest/Tools/bin/llvm-ranlib
    CACHE FILEPATH "Hexagon LLLVM ranlib")
set(CMAKE_NM
    /prj/qct/llvm/release/internal/HEXAGON/${BRANCH}/linux64/latest/Tools/bin/hexagon-nm
    CACHE FILEPATH "Hexagon LLLVM nm")
