/**
 * Couche DB — connexion SQLite, exécution requêtes, gestion erreurs.
 * Chemin fourni par l'appelant (TASKMAN_DB_NAME via main).
 * En cas d'erreur : stderr + retour false (appelant retourne code 1).
 */

#ifndef TASKMAN_DB_HPP
#define TASKMAN_DB_HPP

#include <map>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace taskman {

class Database {
public:
    Database() : db_(nullptr) {}
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /** Ouvre la base au chemin donné. Crée le fichier si nécessaire.
     * En échec : message sur stderr, retour false. */
    bool open(const char* path);

    /** Ferme la connexion. No-op si déjà fermée. */
    void close();

    /** Exécute une requête SQL (DDL/DML).
     * En échec : message sur stderr, retour false. */
    bool exec(const char* sql);

    /** Exécute une requête paramétrée (?, ?, …). params[i] = nullopt → bind NULL.
     * En échec : message sur stderr, retour false. */
    bool run(const char* sql, const std::vector<std::optional<std::string>>& params);

    /** SELECT : retourne les lignes en map nom_colonne → valeur (nullopt = SQL NULL).
     * En échec : message sur stderr, retour vector vide. */
    std::vector<std::map<std::string, std::optional<std::string>>> query(const char* sql);

    /** Vrai si une connexion est ouverte. */
    bool is_open() const { return db_ != nullptr; }

private:
    struct sqlite3* db_;
};

/** Crée les 4 tables (phases, milestones, tasks, task_deps).
 * Retourne false en cas d'erreur (stderr déjà écrit par exec). */
bool init_schema(Database& db);

} // namespace taskman

#endif /* TASKMAN_DB_HPP */
