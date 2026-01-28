/**
 * SchemaManager — gestion du schéma de base de données uniquement.
 * Responsabilité unique : initialisation et migrations du schéma.
 * Nécessite un QueryExecutor pour fonctionner.
 */

#ifndef TASKMAN_SCHEMA_MANAGER_HPP
#define TASKMAN_SCHEMA_MANAGER_HPP

#include "query_executor.hpp"

namespace taskman {

class SchemaManager {
public:
    /** Constructeur prenant une référence à QueryExecutor.
     * La connexion doit être ouverte avant d'utiliser SchemaManager. */
    explicit SchemaManager(QueryExecutor& executor) : executor_(executor) {}

    SchemaManager(const SchemaManager&) = delete;
    SchemaManager& operator=(const SchemaManager&) = delete;

    /** Crée les tables (phases, milestones, tasks, task_deps, task_notes).
     * Retourne false en cas d'erreur (stderr déjà écrit par executor). */
    bool init_schema();

private:
    QueryExecutor& executor_;

    /** Vérifie si une table a une colonne donnée. */
    bool table_has_column(const char* table, const char* column);

    /** Assure que les colonnes created_at et updated_at existent dans une table. */
    bool ensure_timestamps(const char* table);
};

} // namespace taskman

#endif /* TASKMAN_SCHEMA_MANAGER_HPP */
