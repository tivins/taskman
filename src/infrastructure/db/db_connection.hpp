/**
 * DatabaseConnection — gestion de la connexion SQLite uniquement.
 * Responsabilité unique : ouvrir/fermer la connexion et vérifier son état.
 */

#ifndef TASKMAN_DB_CONNECTION_HPP
#define TASKMAN_DB_CONNECTION_HPP

struct sqlite3;

namespace taskman {

class DatabaseConnection {
public:
    DatabaseConnection() : db_(nullptr) {}
    ~DatabaseConnection();

    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    /** Ouvre la base au chemin donné. Crée le fichier si nécessaire.
     * En échec : message sur stderr, retour false. */
    bool open(const char* path);

    /** Ferme la connexion. No-op si déjà fermée. */
    void close();

    /** Vrai si une connexion est ouverte. */
    bool is_open() const { return db_ != nullptr; }

    /** Accès à la connexion SQLite brute (pour QueryExecutor).
     * Retourne nullptr si non connecté. */
    sqlite3* get() const { return db_; }

private:
    struct sqlite3* db_;
};

} // namespace taskman

#endif /* TASKMAN_DB_CONNECTION_HPP */
