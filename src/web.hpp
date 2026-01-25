/**
 * Commande taskman web — serveur HTTP pour consultation de la base.
 * GET / : page HTML (CSS + JS module), GET /task/:id, GET /tasks?limit=&page= → JSON.
 */

#ifndef TASKMAN_WEB_HPP
#define TASKMAN_WEB_HPP

namespace taskman {

class Database;

/** taskman web [--host <h>] [--port <p>]
 *  Défaut : 127.0.0.1:8080. Démarre le serveur HTTP (bloquant). */
int cmd_web(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_WEB_HPP */
