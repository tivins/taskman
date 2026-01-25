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
# Rendre le répertoire des en-têtes SQLite disponible au parent (SQLite3 n’expose pas toujours les includes)
set(SQLITE_AMALGAMATION_SOURCE_DIR ${sqlite_amalgamation_SOURCE_DIR} PARENT_SCOPE)
