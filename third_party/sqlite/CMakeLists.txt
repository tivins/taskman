# SQLite : amalgamation via FetchContent (azadkuh/sqlite-amalgamation)
# Expose la cible SQLite3 (lib statique sqlite3.c + sqlite3.h).

include(FetchContent)
FetchContent_Declare(
  sqlite_amalgamation
  GIT_REPOSITORY https://github.com/azadkuh/sqlite-amalgamation.git
  GIT_TAG        master
)
set(BUILD_SHELL OFF CACHE BOOL "Skip sqlite3 shell" FORCE)
FetchContent_MakeAvailable(sqlite_amalgamation)
