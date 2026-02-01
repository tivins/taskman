/**
 * Gestion de l'état listé dans l'URL (ADR-0002).
 * Responsabilité unique : lire/écrire l'état de la vue liste dans le hash (#/list?page=...).
 * Format : #/list?page=2&phase=P1&status=in_progress&search=...&sort=...&order=...&group_by=...
 */

const LIST_PATH = '/list';
const OVERVIEW_PATH = '/overview';
const BOARD_PATH = '/board';
const TASK_PATH_PREFIX = '/task/';

const LIST_STATE_KEYS = [
    'page', 'phase', 'milestone', 'role', 'status', 'blocked_filter',
    'search', 'sort', 'order', 'group_by', 'page_size'
];

/**
 * Parse le hash actuel en { path, pathParams, queryParams }.
 * pathParams : pour #/task/:id → { id }
 * @returns {{ path: string, pathParams: Record<string, string>, queryParams: URLSearchParams }}
 */
export function parseHash() {
    const hash = (typeof location !== 'undefined' && location.hash) ? location.hash.slice(1) : '';
    const [pathPart, queryPart] = hash.split('?');
    const path = pathPart || '/';
    const queryParams = new URLSearchParams(queryPart || '');

    let pathParams = {};
    if (path.startsWith(TASK_PATH_PREFIX)) {
        const id = path.slice(TASK_PATH_PREFIX.length).replace(/\/.*$/, '');
        if (id) pathParams.id = id;
    }

    return { path, pathParams, queryParams };
}

/**
 * Récupère l'état liste depuis l'URL (pour la vue list).
 * @returns {Record<string, string|number>}
 */
export function getListStateFromUrl() {
    const { path, queryParams } = parseHash();
    if (path !== LIST_PATH && path !== '/') {
        return {};
    }
    const state = {};
    for (const key of LIST_STATE_KEYS) {
        const val = queryParams.get(key);
        if (val !== null && val !== '') {
            if (key === 'page' || key === 'page_size') {
                const n = parseInt(val, 10);
                if (!Number.isNaN(n)) state[key] = n;
            } else {
                state[key] = val;
            }
        }
    }
    return state;
}

/**
 * Construit la chaîne hash pour la vue liste avec les paramètres donnés.
 * @param {Record<string, string|number>} state - État liste (filtres, page, search, sort, order, group_by, page_size)
 * @returns {string}
 */
export function buildListHash(state) {
    const params = new URLSearchParams();
    for (const key of LIST_STATE_KEYS) {
        const val = state[key];
        if (val !== undefined && val !== null && val !== '') {
            params.set(key, String(val));
        }
    }
    const query = params.toString();
    return LIST_PATH + (query ? '?' + query : '');
}

/**
 * Met à jour l'URL avec l'état liste (pushState pour navigation, replaceState pour correction silencieuse).
 * @param {Record<string, string|number>} state
 * @param {boolean} replace - true = replaceState, false = pushState
 */
export function setListStateInUrl(state, replace = false) {
    const hash = '#' + buildListHash(state);
    if (typeof history !== 'undefined') {
        if (replace) {
            history.replaceState(null, '', hash);
        } else {
            history.pushState(null, '', hash);
        }
    }
}

/**
 * Retourne la vue courante à partir du path du hash.
 * @returns {'list'|'overview'|'board'|'task'|null}
 */
export function getViewFromUrl() {
    const { path } = parseHash();
    if (path === LIST_PATH || path === '/') return 'list';
    if (path === OVERVIEW_PATH) return 'overview';
    if (path === BOARD_PATH) return 'board';
    if (path.startsWith(TASK_PATH_PREFIX)) return 'task';
    return null;
}

/**
 * Retourne l'ID tâche si l'URL est #/task/:id.
 * @returns {string|null}
 */
export function getTaskIdFromUrl() {
    const { pathParams } = parseHash();
    return pathParams.id || null;
}

/**
 * Met l'URL sur la route tâche #/task/:id (pour partage / deep link — ADR-0002).
 * @param {string} id - ID de la tâche
 */
export function setTaskInUrl(id) {
    if (typeof history === 'undefined' || !id) return;
    const hash = '#' + TASK_PATH_PREFIX + encodeURIComponent(id);
    history.pushState(null, '', hash);
}

/**
 * Récupère l'état board depuis l'URL (#/board?swimlane=...).
 * @returns {{ swimlane: string }}
 */
export function getBoardStateFromUrl() {
    const { path, queryParams } = parseHash();
    if (path !== BOARD_PATH) return { swimlane: '' };
    const swimlane = queryParams.get('swimlane') || '';
    if (['phase', 'role', 'milestone'].indexOf(swimlane) === -1) return { swimlane: '' };
    return { swimlane };
}

/**
 * Met l'URL sur la vue board avec état optionnel (swimlane).
 * @param {{ swimlane?: string }} state
 * @param {boolean} replace
 */
export function setBoardInUrl(state = {}, replace = false) {
    const params = new URLSearchParams();
    if (state.swimlane) params.set('swimlane', state.swimlane);
    const query = params.toString();
    const hash = '#' + BOARD_PATH + (query ? '?' + query : '');
    if (typeof history !== 'undefined') {
        if (replace) history.replaceState(null, '', hash);
        else history.pushState(null, '', hash);
    }
}

/**
 * Enregistre un listener pour les changements de navigation (back/forward).
 * @param {() => void} callback
 */
export function onPopState(callback) {
    if (typeof window !== 'undefined') {
        window.addEventListener('popstate', callback);
    }
}
