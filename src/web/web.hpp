/**
 * Commande taskman web — serveur HTTP pour consultation de la base.
 * GET / : page HTML (CSS + JS module), GET /task/:id, GET /task/:id/notes, GET /tasks?limit=&page= → JSON.
 */

#ifndef TASKMAN_WEB_HPP
#define TASKMAN_WEB_HPP

namespace taskman {

class Database;

/** taskman web [--host <h>] [--port <p>] [--serve-assets-from <dir>]
 *  Défaut : 127.0.0.1:8080. --serve-assets-from : en dev, servir CSS/JS depuis <dir> (sans recompile). */
int cmd_web(int argc, char* argv[], Database& db);

} // namespace taskman

#endif /* TASKMAN_WEB_HPP */
