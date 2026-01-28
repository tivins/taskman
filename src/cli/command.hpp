/**
 * Pattern Command pour le routage des commandes CLI.
 * Respecte le principe Open/Closed : nouvelles commandes sans modifier main.cpp.
 */

#ifndef TASKMAN_COMMAND_HPP
#define TASKMAN_COMMAND_HPP

#include <memory>
#include <string>
#include <map>

namespace taskman {

class Database;

/**
 * Interface abstraite pour une commande CLI.
 * Respecte le principe Open/Closed : nouvelles commandes héritent de Command.
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * Exécute la commande avec les arguments fournis.
     * @param argc Nombre d'arguments
     * @param argv Tableau d'arguments
     * @param db Référence à la base de données (peut être nullptr pour certaines commandes)
     * @return Code de sortie (0 = succès, != 0 = erreur)
     */
    virtual int execute(int argc, char* argv[], Database* db) = 0;

    /**
     * Retourne le nom de la commande (ex: "task:add").
     */
    virtual std::string name() const = 0;

    /**
     * Retourne un résumé de la commande pour l'aide.
     */
    virtual std::string summary() const = 0;

    /**
     * Indique si la commande nécessite une base de données ouverte.
     * Si false, db peut être nullptr dans execute().
     */
    virtual bool requires_database() const { return true; }
};

/**
 * Registre de commandes.
 * Permet d'enregistrer et d'exécuter des commandes par nom.
 * Respecte le principe Open/Closed : nouvelles commandes s'enregistrent sans modifier le code existant.
 */
class CommandRegistry {
public:
    CommandRegistry();
    ~CommandRegistry() = default;

    /**
     * Enregistre une commande dans le registre.
     * @param cmd Commande à enregistrer (le registre prend possession)
     */
    void register_command(std::unique_ptr<Command> cmd);

    /**
     * Exécute une commande par son nom.
     * @param name Nom de la commande
     * @param argc Nombre d'arguments
     * @param argv Tableau d'arguments
     * @param db Référence à la base de données
     * @return Code de sortie, ou -1 si la commande n'existe pas
     */
    int execute(const std::string& name, int argc, char* argv[], Database* db) const;

    /**
     * Retourne toutes les commandes enregistrées avec leurs résumés.
     * Utile pour générer l'aide.
     */
    std::map<std::string, std::string> list_commands() const;

    /**
     * Vérifie si une commande existe.
     */
    bool has_command(const std::string& name) const;

    /**
     * Vérifie si une commande nécessite une base de données.
     */
    bool command_requires_database(const std::string& name) const;

private:
    std::map<std::string, std::unique_ptr<Command>> commands_;
};

/**
 * Enregistre toutes les commandes disponibles dans le registre.
 * Cette fonction centralise l'enregistrement de toutes les commandes.
 */
void register_all_commands(CommandRegistry& registry);

} // namespace taskman

#endif /* TASKMAN_COMMAND_HPP */
