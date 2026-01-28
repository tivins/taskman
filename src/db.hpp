/**
 * Couche DB — connexion SQLite, exécution requêtes, gestion erreurs.
 * Chemin fourni par l'appelant (TASKMAN_DB_NAME via main).
 * En cas d'erreur : stderr + retour false (appelant retourne code 1).
 * 
 * Cette classe est une façade qui délègue aux classes spécialisées :
 * - DatabaseConnection : gestion de la connexion
 * - QueryExecutor : exécution de requêtes
 * - SchemaManager : gestion du schéma
 * 
 * Cette architecture respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_DB_HPP
#define TASKMAN_DB_HPP

#include "db_connection.hpp"
#include "query_executor.hpp"
#include "schema_manager.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace taskman {

class Database {
public:
    Database() : connection_(), executor_(connection_), schema_manager_(executor_) {}
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /** Ouvre la base au chemin donné. Crée le fichier si nécessaire.
     * En échec : message sur stderr, retour false. */
    bool open(const char* path) { return connection_.open(path); }

    /** Ferme la connexion. No-op si déjà fermée. */
    void close() { connection_.close(); }

    /** Exécute une requête SQL (DDL/DML).
     * En échec : message sur stderr, retour false. */
    bool exec(const char* sql) { return executor_.exec(sql); }

    /** Exécute une requête paramétrée (?, ?, …). params[i] = nullopt → bind NULL.
     * En échec : message sur stderr, retour false. */
    bool run(const char* sql, const std::vector<std::optional<std::string>>& params) {
        return executor_.run(sql, params);
    }

    /** SELECT : retourne les lignes en map nom_colonne → valeur (nullopt = SQL NULL).
     * En échec : message sur stderr, retour vector vide. */
    std::vector<std::map<std::string, std::optional<std::string>>> query(const char* sql) {
        return executor_.query(sql);
    }

    /** SELECT paramétré (?, ?, …). params[i] = nullopt → bind NULL. */
    std::vector<std::map<std::string, std::optional<std::string>>> query(
        const char* sql, const std::vector<std::optional<std::string>>& params) {
        return executor_.query(sql, params);
    }

    /** Vrai si une connexion est ouverte. */
    bool is_open() const { return connection_.is_open(); }

    /** Initialise le schéma de la base de données.
     * Retourne false en cas d'erreur (stderr déjà écrit par executor). */
    bool init_schema() { return schema_manager_.init_schema(); }

private:
    DatabaseConnection connection_;
    QueryExecutor executor_;
    SchemaManager schema_manager_;
};

} // namespace taskman

#endif /* TASKMAN_DB_HPP */
