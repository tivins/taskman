/**
 * Implémentation du pattern Command pour le routage des commandes CLI.
 */

#include "command.hpp"
#include <stdexcept>

namespace taskman {

CommandRegistry::CommandRegistry() = default;

void CommandRegistry::register_command(std::unique_ptr<Command> cmd) {
    if (!cmd) {
        throw std::invalid_argument("Cannot register null command");
    }
    std::string name = cmd->name();
    commands_[name] = std::move(cmd);
}

int CommandRegistry::execute(const std::string& name, int argc, char* argv[], Database* db) const {
    auto it = commands_.find(name);
    if (it == commands_.end()) {
        return -1; // Commande non trouvée
    }
    return it->second->execute(argc, argv, db);
}

std::map<std::string, std::string> CommandRegistry::list_commands() const {
    std::map<std::string, std::string> result;
    for (const auto& [name, cmd] : commands_) {
        result[name] = cmd->summary();
    }
    return result;
}

bool CommandRegistry::has_command(const std::string& name) const {
    return commands_.find(name) != commands_.end();
}

bool CommandRegistry::command_requires_database(const std::string& name) const {
    auto it = commands_.find(name);
    if (it == commands_.end()) {
        return false; // Commande inexistante, par défaut pas de DB nécessaire
    }
    return it->second->requires_database();
}

} // namespace taskman
