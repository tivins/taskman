/**
 * Contrôleurs REST pour l'API web.
 * Chaque contrôleur gère les endpoints pour une ressource spécifique.
 * Respecte le principe SRP (Single Responsibility Principle).
 */

#ifndef TASKMAN_WEB_CONTROLLERS_HPP
#define TASKMAN_WEB_CONTROLLERS_HPP

#include <httplib.h>
#include <string>

namespace taskman {

class TaskRepository;
class TaskService;
class PhaseRepository;
class MilestoneRepository;
class NoteRepository;

/**
 * Contrôleur pour les endpoints de tâches.
 * Responsabilité unique : gestion des requêtes HTTP pour les tâches.
 */
class TaskController {
public:
    TaskController(TaskRepository& task_repo, TaskService& task_service, NoteRepository& note_repo);

    /** Enregistre les routes de tâches sur le serveur HTTP. */
    void register_routes(httplib::Server& svr);

private:
    TaskRepository& task_repo_;
    TaskService& task_service_;
    NoteRepository& note_repo_;
};

/**
 * Contrôleur pour les endpoints de phases.
 * Responsabilité unique : gestion des requêtes HTTP pour les phases.
 */
class PhaseController {
public:
    explicit PhaseController(PhaseRepository& phase_repo);

    /** Enregistre les routes de phases sur le serveur HTTP. */
    void register_routes(httplib::Server& svr);

private:
    PhaseRepository& phase_repo_;
};

/**
 * Contrôleur pour les endpoints de milestones.
 * Responsabilité unique : gestion des requêtes HTTP pour les milestones.
 */
class MilestoneController {
public:
    explicit MilestoneController(MilestoneRepository& milestone_repo);

    /** Enregistre les routes de milestones sur le serveur HTTP. */
    void register_routes(httplib::Server& svr);

private:
    MilestoneRepository& milestone_repo_;
};

} // namespace taskman

#endif /* TASKMAN_WEB_CONTROLLERS_HPP */
