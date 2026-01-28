/**
 * QueryExecutor — exécution de requêtes SQL uniquement.
 * Responsabilité unique : exécuter des requêtes SQL (DDL, DML, SELECT).
 * Nécessite une DatabaseConnection pour fonctionner.
 */

#ifndef TASKMAN_QUERY_EXECUTOR_HPP
#define TASKMAN_QUERY_EXECUTOR_HPP

#include "db_connection.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class QueryExecutor {
public:
    /** Constructeur prenant une référence à DatabaseConnection.
     * La connexion doit être ouverte avant d'utiliser QueryExecutor. */
    explicit QueryExecutor(DatabaseConnection& connection) : connection_(connection) {}

    QueryExecutor(const QueryExecutor&) = delete;
    QueryExecutor& operator=(const QueryExecutor&) = delete;

    /** Exécute une requête SQL (DDL/DML).
     * En échec : message sur stderr, retour false. */
    bool exec(const char* sql);

    /** Exécute une requête paramétrée (?, ?, …). params[i] = nullopt → bind NULL.
     * En échec : message sur stderr, retour false. */
    bool run(const char* sql, const std::vector<std::optional<std::string>>& params);

    /** SELECT : retourne les lignes en map nom_colonne → valeur (nullopt = SQL NULL).
     * En échec : message sur stderr, retour vector vide. */
    std::vector<std::map<std::string, std::optional<std::string>>> query(const char* sql);

    /** SELECT paramétré (?, ?, …). params[i] = nullopt → bind NULL. */
    std::vector<std::map<std::string, std::optional<std::string>>> query(
        const char* sql, const std::vector<std::optional<std::string>>& params);

private:
    DatabaseConnection& connection_;
};

} // namespace taskman

#endif /* TASKMAN_QUERY_EXECUTOR_HPP */
