import { el } from './dom.js';
import { Filters, ROLE_OPTIONS } from './filters.js';
import { Pagination } from './pagination.js';
import * as urlState from './url-state.js';

const app = document.getElementById('app');

/** @type {HTMLElement | null} zone principale du shell (header + main) */
let appMain = null;

// État global
let filters = new Filters();
let pagination = new Pagination();
/** @type {Record<string, { id: string, name?: string, [k: string]: unknown }>} phases indexés par id */
let phases = {};
/** @type {Record<string, { id: string, name?: string, [k: string]: unknown }>} milestones indexés par id */
let milestones = {};
let tasks = [];
let taskDeps = [];

/** État liste (étape 4) : recherche, tri, groupement — persistance URL (ADR-0002) */
let searchQuery = '';
/** Colonne de tri : title | status | role | phase | milestone | updated_at */
let sortBy = 'updated_at';
/** Direction : asc | desc */
let sortOrder = 'desc';
/** Grouper par : phase | milestone | status | role | '' */
let groupBy = '';

/** Flag pour éviter de déclencher loadTasks pendant l'application de l'état depuis l'URL (popstate) */
let applyingStateFromUrl = false;

/**
 * Construit l'état liste actuel (filtres, page, search, sort, group_by) et le reflète dans l'URL (ADR-0002).
 * @param {boolean} replace - true = replaceState (pas d'entrée historique)
 */
function syncListStateToUrl(replace = false) {
    const state = {
        ...filters.getFilters(),
        page: pagination.getCurrentPage(),
        page_size: pagination.getPageSize(),
        search: searchQuery || undefined,
        sort: sortBy !== 'updated_at' ? sortBy : undefined,
        order: sortOrder !== 'desc' ? sortOrder : undefined,
        group_by: groupBy || undefined
    };
    urlState.setListStateInUrl(state, replace);
}

/**
 * Met à jour le champ de recherche du header avec la valeur courante de searchQuery.
 */
function updateHeaderSearchInput() {
    const input = app.querySelector('.app-search-input');
    if (input) input.value = searchQuery;
}

/**
 * Met à jour les selects tri / ordre / groupement avec l'état courant.
 */
function updateListOptionsFromState() {
    const sortSelect = document.getElementById('list-sort');
    const orderSelect = document.getElementById('list-order');
    const groupSelect = document.getElementById('list-group-by');
    if (sortSelect) sortSelect.value = sortBy;
    if (orderSelect) orderSelect.value = sortOrder;
    if (groupSelect) groupSelect.value = groupBy;
}

/**
 * Applique l'état lu depuis l'URL aux filtres, pagination et état local (search, sort, groupBy).
 * @param {Record<string, string|number>} state
 */
function applyListStateFromUrl(state) {
    if (state.phase !== undefined) filters.setFilter('phase', String(state.phase));
    if (state.milestone !== undefined) filters.setFilter('milestone', String(state.milestone));
    if (state.role !== undefined) filters.setFilter('role', String(state.role));
    if (state.status !== undefined) filters.setFilter('status', String(state.status));
    if (state.blocked_filter !== undefined) filters.setFilter('blocked_filter', String(state.blocked_filter));
    if (state.page !== undefined) pagination.goToPage(Number(state.page));
    if (state.page_size !== undefined) pagination.setPageSize(Number(state.page_size));
    if (state.search !== undefined) searchQuery = String(state.search).trim();
    if (state.sort !== undefined) sortBy = String(state.sort);
    if (state.order !== undefined) sortOrder = String(state.order) === 'asc' ? 'asc' : 'desc';
    if (state.group_by !== undefined) groupBy = String(state.group_by);
}

/**
 * Initialisation de l'application
 */
async function init() {
    // Charger les phases et milestones pour les filtres
    await loadPhasesAndMilestones();
    updateSidebarTree();

    // Restaurer l'état liste depuis l'URL au chargement (ADR-0002)
    const view = urlState.getViewFromUrl();
    const listState = urlState.getListStateFromUrl();
    if (Object.keys(listState).length > 0) {
        applyListStateFromUrl(listState);
    }
    syncListStateToUrl(true);

    // Configurer les callbacks
    filters.setOnChange(() => {
        if (applyingStateFromUrl) return;
        pagination.goToPage(1);
        syncListStateToUrl();
        loadTasks();
    });

    pagination.setOnPageChange(() => {
        if (applyingStateFromUrl) return;
        syncListStateToUrl();
        loadTasks();
    });

    // Navigation back/forward : réappliquer l'état depuis l'URL (ADR-0002)
    urlState.onPopState(() => {
        applyingStateFromUrl = true;
        const listStateFromUrl = urlState.getListStateFromUrl();
        applyListStateFromUrl(listStateFromUrl);
        updateHeaderSearchInput();
        updateListOptionsFromState();
        applyingStateFromUrl = false;
        const v = urlState.getViewFromUrl();
        if (v === 'list') {
            closePeek();
            setActiveNav('tasks');
            renderMainView();
            loadTasks();
        } else if (v === 'overview') {
            closePeek();
            setActiveNav('overview');
            showDashboard();
        } else if (v === 'task') {
            setActiveNav('tasks');
            renderMainView();
            loadTasks().then(() => {
                const taskId = urlState.getTaskIdFromUrl();
                if (taskId) openPeek(taskId, { fromUrl: true });
            });
        }
    });

    // Shell (header fixe + zone principale) puis vue liste
    ensureAppShell();
    setActiveNav(view === 'overview' ? 'overview' : 'tasks');
    if (view === 'overview') {
        showDashboard();
    } else {
        renderMainView();
        await loadTasks();
        if (view === 'task') {
            const taskId = urlState.getTaskIdFromUrl();
            if (taskId) openPeek(taskId, { fromUrl: true });
        }
    }
}

/** Clé localStorage pour l’état replié de la sidebar */
const SIDEBAR_COLLAPSED_KEY = 'taskman-sidebar-collapsed';

/**
 * Crée le shell commun (header fixe + body avec sidebar + zone principale) si nécessaire.
 * À appeler avant toute vue.
 */
function ensureAppShell() {
    if (app.querySelector('.app-shell')) return;
    app.innerHTML = '';

    const shell = el('div', { class: 'app-shell' });

    const header = el('div', { class: 'app-header' });

    header.appendChild(el('div', { class: 'app-brand' }, 'Taskman'));
    const nav = el('nav', { class: 'app-nav' });
    nav.appendChild(el('a', { href: '#/list', class: 'app-nav-link app-nav-link-active', 'data-view': 'tasks' }, 'Tâches'));
    nav.appendChild(el('a', { href: '#/overview', class: 'app-nav-link', 'data-view': 'overview' }, 'Vue d\'ensemble'));
    header.appendChild(nav);
    const headerActions = el('div', { class: 'app-header-actions' });
    // Recherche globale (étape 4.1 — ADR-0003 : temporaire côté client tant que l’API n’expose pas search)
    const searchWrap = el('div', { class: 'app-search-wrap' });
    const searchInput = el('input', {
        type: 'search',
        class: 'app-search-input',
        placeholder: 'Rechercher (titre, description, ID)…',
        'aria-label': 'Recherche globale'
    });
    searchInput.value = searchQuery;
    let searchDebounce = null;
    searchInput.addEventListener('input', () => {
        if (searchDebounce) clearTimeout(searchDebounce);
        searchDebounce = setTimeout(() => {
            searchQuery = (searchInput.value || '').trim();
            syncListStateToUrl();
            pagination.goToPage(1);
            loadTasks();
            searchDebounce = null;
        }, 300);
    });
    searchWrap.appendChild(searchInput);
    headerActions.appendChild(searchWrap);
    header.appendChild(headerActions);
    shell.appendChild(header);

    const body = el('div', { class: 'app-body' });
    const sidebar = createSidebar();
    body.appendChild(sidebar);

    const main = el('div', { class: 'app-main' });
    body.appendChild(main);

    const peek = el('div', { class: 'app-peek', id: 'app-peek', 'aria-label': 'Détail de la tâche' });
    peek.appendChild(el('div', { class: 'app-peek-inner' }));
    body.appendChild(peek);

    shell.appendChild(body);

    app.appendChild(shell);
    appMain = main;

    const collapsed = localStorage.getItem(SIDEBAR_COLLAPSED_KEY) === 'true';
    if (collapsed) {
        sidebar.classList.add('app-sidebar--collapsed');
        const toggleBtn = sidebar.querySelector('.app-sidebar-toggle');
        if (toggleBtn) toggleBtn.textContent = '▶';
    }

    nav.querySelectorAll('.app-nav-link').forEach((link) => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            const view = link.getAttribute('data-view');
            setActiveNav(view);
            if (view === 'tasks') {
                syncListStateToUrl();
                renderMainView();
                loadTasks();
            } else if (view === 'overview') {
                if (typeof history !== 'undefined') history.pushState(null, '', '#/overview');
                showDashboard();
            }
        });
    });
}

/**
 * Crée la sidebar gauche (repliable) : lien replier/déplier, liens rapides, arborescence phases/milestones.
 */
function createSidebar() {
    const sidebar = el('div', { class: 'app-sidebar', id: 'app-sidebar' });

    const toggleBtn = el('button', {
        type: 'button',
        class: 'app-sidebar-toggle',
        'aria-label': 'Replier / Déplier la barre latérale',
        title: 'Replier / Déplier'
    }, '◀');
    toggleBtn.addEventListener('click', () => {
        sidebar.classList.toggle('app-sidebar--collapsed');
        const collapsed = sidebar.classList.contains('app-sidebar--collapsed');
        localStorage.setItem(SIDEBAR_COLLAPSED_KEY, collapsed);
        toggleBtn.textContent = collapsed ? '▶' : '◀';
    });
    sidebar.appendChild(toggleBtn);

    const content = el('div', { class: 'app-sidebar-content' });

    const quickLinks = el('nav', { class: 'app-sidebar-quicklinks' });
    quickLinks.appendChild(el('a', { href: '#/list', class: 'app-sidebar-link', 'data-view': 'tasks' }, 'Tâches'));
    quickLinks.appendChild(el('a', { href: '#/overview', class: 'app-sidebar-link', 'data-view': 'overview' }, 'Vue d\'ensemble'));
    content.appendChild(quickLinks);

    const treeTitle = el('h3', { class: 'app-sidebar-tree-title' }, 'Phases & jalons');
    content.appendChild(treeTitle);
    const treeContainer = el('div', { class: 'app-sidebar-tree', id: 'app-sidebar-tree' });
    content.appendChild(treeContainer);

    sidebar.appendChild(content);

    quickLinks.querySelectorAll('.app-sidebar-link').forEach((link) => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            const view = link.getAttribute('data-view');
            setActiveNav(view);
            if (view === 'tasks') {
                syncListStateToUrl();
                renderMainView();
                loadTasks();
            } else if (view === 'overview') {
                if (typeof history !== 'undefined') history.pushState(null, '', '#/overview');
                showDashboard();
            }
        });
    });

    return sidebar;
}

/**
 * Met à jour l’arborescence phases/milestones dans la sidebar (à appeler après chargement des données).
 */
function updateSidebarTree() {
    const treeContainer = document.getElementById('app-sidebar-tree');
    if (!treeContainer) return;

    treeContainer.innerHTML = '';
    const phaseList = Object.values(phases);
    const milestonesByPhase = {};
    for (const m of Object.values(milestones)) {
        const pid = m.phase_id || '';
        if (!milestonesByPhase[pid]) milestonesByPhase[pid] = [];
        milestonesByPhase[pid].push(m);
    }

    for (const p of phaseList) {
        const phaseItem = el('div', { class: 'app-sidebar-tree-phase' });
        const phaseLink = el('button', { type: 'button', class: 'app-sidebar-tree-phase-btn', 'data-phase-id': p.id }, escapeHtml(p.name || p.id));
        phaseLink.addEventListener('click', () => {
            filters.setFilter('phase', p.id);
            filters.setFilter('milestone', '');
            setActiveNav('tasks');
            renderMainView();
            loadTasks();
        });
        phaseItem.appendChild(phaseLink);
        const ms = milestonesByPhase[p.id] || [];
        if (ms.length) {
            const sub = el('div', { class: 'app-sidebar-tree-milestones' });
            for (const m of ms) {
                const ml = el('button', { type: 'button', class: 'app-sidebar-tree-milestone-btn', 'data-milestone-id': m.id }, escapeHtml(m.name || m.id));
                ml.addEventListener('click', () => {
                    filters.setFilter('phase', p.id);
                    filters.setFilter('milestone', m.id);
                    setActiveNav('tasks');
                    renderMainView();
                    loadTasks();
                });
                sub.appendChild(ml);
            }
            phaseItem.appendChild(sub);
        }
        treeContainer.appendChild(phaseItem);
    }

    if (phaseList.length === 0) {
        treeContainer.appendChild(el('p', { class: 'muted app-sidebar-tree-empty' }, 'Aucune phase.'));
    }
}

/**
 * Met à jour le lien actif dans la navigation
 */
function setActiveNav(view) {
    const nav = app.querySelector('.app-nav');
    if (!nav) return;
    nav.querySelectorAll('.app-nav-link').forEach((link) => {
        const linkView = link.getAttribute('data-view');
        link.classList.toggle('app-nav-link-active', linkView === view);
    });
}

/**
 * Charge les phases et milestones pour les filtres
 */
async function loadPhasesAndMilestones() {
    try {
        const [phasesRes, milestonesRes] = await Promise.all([
            fetch('/phases?limit=100'),
            fetch('/milestones?limit=100')
        ]);
        
        if (phasesRes.ok) {
            const arr = await phasesRes.json();
            phases = Object.fromEntries((Array.isArray(arr) ? arr : []).map((p) => [p.id, p]));
        }
        
        if (milestonesRes.ok) {
            const arr = await milestonesRes.json();
            milestones = Object.fromEntries((Array.isArray(arr) ? arr : []).map((m) => [m.id, m]));
        }
    } catch (e) {
        console.error('Error loading phases/milestones:', e);
    }
}

/**
 * Affiche la vue principale (liste) avec filtres, tri, groupement et pagination.
 */
function renderMainView() {
    if (!appMain) ensureAppShell();
    const main = appMain || app.querySelector('.app-main');
    if (!main) return;
    main.innerHTML = '';

    // Filtres
    const filtersContainer = document.createElement('div');
    filtersContainer.className = 'filters-wrapper';
    filters.render(filtersContainer);
    main.appendChild(filtersContainer);

    const phaseOptions = [].concat(
        Object.values(phases).map(p => ({ value: p.id, label: `${p.id}: ${p.name || p.id}` }))
    );
    filters.updateSelectOptions('phase', phaseOptions);

    const milestoneOptions = [].concat(
        Object.values(milestones).map(m => ({ value: m.id, label: `${m.id}: ${m.name || m.id}` }))
    );
    filters.updateSelectOptions('milestone', milestoneOptions);

    // Tri et groupement (étape 4.2, 4.3)
    const listOptionsDiv = el('div', { class: 'list-options' });
    const sortLabel = el('label', { for: 'list-sort' }, 'Tri : ');
    const sortSelect = el('select', { id: 'list-sort', class: 'list-option-select' });
    const sortColumns = [
        { value: 'title', label: 'Titre' },
        { value: 'status', label: 'Statut' },
        { value: 'role', label: 'Rôle' },
        { value: 'phase_id', label: 'Phase' },
        { value: 'milestone_id', label: 'Jalon' },
        { value: 'updated_at', label: 'Mis à jour' }
    ];
    sortColumns.forEach(({ value, label }) => {
        const opt = el('option', { value }, label);
        if (value === sortBy) opt.selected = true;
        sortSelect.appendChild(opt);
    });
    sortSelect.addEventListener('change', () => {
        sortBy = sortSelect.value;
        syncListStateToUrl();
        if (tasks.length) renderTasksInContent(tasks);
    });

    const orderSelect = el('select', { id: 'list-order', class: 'list-option-select' });
    orderSelect.appendChild(el('option', { value: 'asc' }, 'Ascendant'));
    orderSelect.appendChild(el('option', { value: 'desc' }, 'Descendant'));
    orderSelect.value = sortOrder;
    orderSelect.addEventListener('change', () => {
        sortOrder = orderSelect.value;
        syncListStateToUrl();
        if (tasks.length) renderTasksInContent(tasks);
    });

    const groupLabel = el('label', { for: 'list-group-by' }, ' Grouper par : ');
    const groupSelect = el('select', { id: 'list-group-by', class: 'list-option-select' });
    groupSelect.appendChild(el('option', { value: '' }, '— Aucun —'));
    groupSelect.appendChild(el('option', { value: 'phase' }, 'Phase'));
    groupSelect.appendChild(el('option', { value: 'milestone' }, 'Jalon'));
    groupSelect.appendChild(el('option', { value: 'status' }, 'Statut'));
    groupSelect.appendChild(el('option', { value: 'role' }, 'Rôle'));
    groupSelect.value = groupBy;
    groupSelect.addEventListener('change', () => {
        groupBy = groupSelect.value;
        syncListStateToUrl();
        if (tasks.length) renderTasksInContent(tasks);
    });

    listOptionsDiv.appendChild(sortLabel);
    listOptionsDiv.appendChild(sortSelect);
    listOptionsDiv.appendChild(orderSelect);
    listOptionsDiv.appendChild(groupLabel);
    listOptionsDiv.appendChild(groupSelect);
    updateListOptionsFromState();
    main.appendChild(listOptionsDiv);

    // Zone de contenu pour les tâches (pleine largeur)
    const contentDiv = el('div', { class: 'tasks-content', id: 'tasks-content' });
    main.appendChild(contentDiv);

    // Pagination
    const paginationContainer = document.createElement('div');
    paginationContainer.className = 'pagination-wrapper';
    paginationContainer.id = 'pagination-container';
    main.appendChild(paginationContainer);
}

/**
 * Affiche la vue Dashboard (Vue d'ensemble) dans la zone principale.
 */
function showDashboard() {
    if (!appMain) ensureAppShell();
    const main = appMain || app.querySelector('.app-main');
    if (!main) return;
    main.innerHTML = '';
    const container = el('div', { class: 'dashboard', id: 'dashboard-container' });
    main.appendChild(container);
    renderDashboardGrid(container);
    loadDashboardData(container);
}

/**
 * Crée la grille de widgets du dashboard (structure vide, remplie par loadDashboardData).
 */
function renderDashboardGrid(container) {
    const grid = el('div', { class: 'dashboard-grid' });
    grid.appendChild(el('div', { class: 'dashboard-widget', id: 'dashboard-widget-status' }, el('h3', {}, 'Tâches par statut'), el('p', { class: 'muted' }, 'Chargement…')));
    grid.appendChild(el('div', { class: 'dashboard-widget', id: 'dashboard-widget-roles' }, el('h3', {}, 'Tâches par rôle'), el('p', { class: 'muted' }, 'Chargement…')));
    grid.appendChild(el('div', { class: 'dashboard-widget dashboard-widget-wide', id: 'dashboard-widget-phases' }, el('h3', {}, 'Progression par phase'), el('p', { class: 'muted' }, 'Chargement…')));
    grid.appendChild(el('div', { class: 'dashboard-widget dashboard-widget-wide', id: 'dashboard-widget-milestones' }, el('h3', {}, 'Milestones'), el('p', { class: 'muted' }, 'Chargement…')));
    container.appendChild(grid);
}

/**
 * Charge toutes les données du dashboard en parallèle puis met à jour chaque widget.
 */
async function loadDashboardData(container) {
    try {
        const [phasesRes, milestonesRes, countTotalRes, countToDoRes, countInProgressRes, countDoneRes, ...roleCountRes] = await Promise.all([
            fetch('/phases?limit=100'),
            fetch('/milestones?limit=100'),
            fetch('/tasks/count'),
            fetch('/tasks/count?status=to_do'),
            fetch('/tasks/count?status=in_progress'),
            fetch('/tasks/count?status=done'),
            ...ROLE_OPTIONS.map((r) => fetch(`/tasks/count?role=${encodeURIComponent(r.value)}`))
        ]);

        const phasesList = phasesRes.ok ? await phasesRes.json() : [];
        const milestonesList = milestonesRes.ok ? await milestonesRes.json() : [];
        const total = countTotalRes.ok ? (await countTotalRes.json()).count : 0;
        const toDo = countToDoRes.ok ? (await countToDoRes.json()).count : 0;
        const inProgress = countInProgressRes.ok ? (await countInProgressRes.json()).count : 0;
        const done = countDoneRes.ok ? (await countDoneRes.json()).count : 0;
        const roleCounts = await Promise.all(roleCountRes.map((r) => (r.ok ? r.json() : { count: 0 })));

        const statusData = { total, to_do: toDo, in_progress: inProgress, done };
        const rolesData = ROLE_OPTIONS.map((r, i) => ({ value: r.value, label: r.label, count: roleCounts[i]?.count ?? 0 }));

        const phaseIds = (Array.isArray(phasesList) ? phasesList : []).map((p) => p.id);
        const phaseCountRes = await Promise.all(
            phaseIds.flatMap((id) => [
                fetch(`/tasks/count?phase=${encodeURIComponent(id)}`),
                fetch(`/tasks/count?phase=${encodeURIComponent(id)}&status=done`)
            ])
        );
        const phaseCounts = [];
        for (let i = 0; i < phaseIds.length; i++) {
            const totalRes = phaseCountRes[i * 2];
            const doneRes = phaseCountRes[i * 2 + 1];
            const phaseTotal = totalRes?.ok ? (await totalRes.json()).count : 0;
            const phaseDone = doneRes?.ok ? (await doneRes.json()).count : 0;
            const phase = (Array.isArray(phasesList) ? phasesList : []).find((p) => p.id === phaseIds[i]);
            phaseCounts.push({
                id: phaseIds[i],
                name: phase?.name ?? phaseIds[i],
                status: phase?.status ?? 'to_do',
                total: phaseTotal,
                done: phaseDone
            });
        }

        const milestonesData = (Array.isArray(milestonesList) ? milestonesList : []).map((m) => ({
            id: m.id,
            name: m.name ?? m.id,
            phase_id: m.phase_id,
            reached: !!m.reached
        }));

        const statusEl = container.querySelector('#dashboard-widget-status');
        const rolesEl = container.querySelector('#dashboard-widget-roles');
        const phasesEl = container.querySelector('#dashboard-widget-phases');
        const milestonesEl = container.querySelector('#dashboard-widget-milestones');
        if (statusEl) renderStatusWidget(statusEl, statusData);
        if (rolesEl) renderRolesWidget(rolesEl, rolesData);
        if (phasesEl) renderPhaseProgressWidget(phasesEl, phaseCounts);
        if (milestonesEl) renderMilestonesWidget(milestonesEl, milestonesData);
    } catch (e) {
        console.error('Dashboard load error:', e);
        const p = container?.querySelector('.dashboard-grid .muted');
        if (p) p.textContent = `Erreur: ${e.message}`;
    }
}

/**
 * Remplit le widget « Tâches par statut » (compteurs + barres).
 */
function renderStatusWidget(widget, data) {
    const content = widget.querySelector('.muted') || widget.appendChild(document.createElement('p'));
    widget.replaceChildren(
        el('h3', {}, 'Tâches par statut'),
        el('div', { class: 'dashboard-status-counters' },
            el('div', { class: 'dashboard-counter dashboard-counter-to_do' }, el('span', { class: 'dashboard-counter-value' }, String(data.to_do)), el('span', { class: 'dashboard-counter-label' }, 'À faire')),
            el('div', { class: 'dashboard-counter dashboard-counter-in_progress' }, el('span', { class: 'dashboard-counter-value' }, String(data.in_progress)), el('span', { class: 'dashboard-counter-label' }, 'En cours')),
            el('div', { class: 'dashboard-counter dashboard-counter-done' }, el('span', { class: 'dashboard-counter-value' }, String(data.done)), el('span', { class: 'dashboard-counter-label' }, 'Terminé'))
        ),
        el('div', { class: 'dashboard-status-bars' },
            el('div', { class: 'dashboard-bar-row' }, el('span', { class: 'dashboard-bar-label' }, 'À faire'), el('div', { class: 'dashboard-bar-track' }, el('div', { class: 'dashboard-bar-fill dashboard-bar-to_do', style: `width:${data.total ? (data.to_do / data.total) * 100 : 0}%` }))),
            el('div', { class: 'dashboard-bar-row' }, el('span', { class: 'dashboard-bar-label' }, 'En cours'), el('div', { class: 'dashboard-bar-track' }, el('div', { class: 'dashboard-bar-fill dashboard-bar-in_progress', style: `width:${data.total ? (data.in_progress / data.total) * 100 : 0}%` }))),
            el('div', { class: 'dashboard-bar-row' }, el('span', { class: 'dashboard-bar-label' }, 'Terminé'), el('div', { class: 'dashboard-bar-track' }, el('div', { class: 'dashboard-bar-fill dashboard-bar-done', style: `width:${data.total ? (data.done / data.total) * 100 : 0}%` })))
        ),
        el('p', { class: 'dashboard-total muted' }, `Total: ${data.total} tâche${data.total !== 1 ? 's' : ''}`)
    );
}

/**
 * Remplit le widget « Tâches par rôle » (liste de compteurs).
 */
function renderRolesWidget(widget, data) {
    const list = el('ul', { class: 'dashboard-role-list' });
    for (const r of data.filter((r) => r.count > 0)) {
        list.appendChild(el('li', {}, el('span', { class: 'dashboard-role-label' }, escapeHtml(r.label)), el('span', { class: 'dashboard-role-count' }, String(r.count))));
    }
    widget.replaceChildren(
        el('h3', {}, 'Tâches par rôle'),
        list.children.length ? list : el('p', { class: 'muted' }, 'Aucune tâche par rôle.')
    );
}

/**
 * Remplit le widget « Progression par phase » (barres par phase).
 */
function renderPhaseProgressWidget(widget, phaseCounts) {
    const list = document.createElement('div');
    list.className = 'dashboard-phase-list';
    for (const p of phaseCounts) {
        const pct = p.total ? Math.round((p.done / p.total) * 100) : 0;
        list.appendChild(
            el('div', { class: 'dashboard-phase-row' },
                el('div', { class: 'dashboard-phase-info' },
                    el('span', { class: 'dashboard-phase-name' }, escapeHtml(p.name)),
                    el('span', { class: 'dashboard-phase-count muted' }, `${p.done}/${p.total}`)
                ),
                el('div', { class: 'dashboard-bar-track' }, el('div', { class: `dashboard-bar-fill dashboard-bar-done`, style: `width:${pct}%` }))
            )
        );
    }
    widget.replaceChildren(
        el('h3', {}, 'Progression par phase'),
        phaseCounts.length ? list : el('p', { class: 'muted' }, 'Aucune phase.')
    );
}

/**
 * Remplit le widget « Milestones » (reached / not reached).
 */
function renderMilestonesWidget(widget, milestonesData) {
    const reached = milestonesData.filter((m) => m.reached);
    const notReached = milestonesData.filter((m) => !m.reached);
    const list = el('div', { class: 'dashboard-milestones-list' });
    if (reached.length) {
        list.appendChild(el('h4', { class: 'dashboard-milestones-subtitle' }, 'Atteints'));
        reached.forEach((m) => list.appendChild(el('div', { class: 'milestone-item reached' }, el('span', {}, escapeHtml(m.name)), el('span', { class: 'reached-badge' }, '✓'))));
    }
    if (notReached.length) {
        list.appendChild(el('h4', { class: 'dashboard-milestones-subtitle' }, 'Non atteints'));
        notReached.forEach((m) => list.appendChild(el('div', { class: 'milestone-item' }, el('span', {}, escapeHtml(m.name)))));
    }
    widget.replaceChildren(
        el('h3', {}, 'Milestones'),
        milestonesData.length ? list : el('p', { class: 'muted' }, 'Aucun milestone.')
    );
}

/**
 * Charge les tâches avec filtres et pagination
 */
async function loadTasks() {
    const contentDiv = document.getElementById('tasks-content');
    if (!contentDiv) return;
    
    contentDiv.innerHTML = '<p>Chargement...</p>';
    
    try {
        const filterParams = filters.buildQueryParams();
        filterParams.append('limit', pagination.getPageSize());
        filterParams.append('page', pagination.getCurrentPage());
        
        // Charger le nombre total et les tâches
        const countParams = filters.buildQueryParams();
        const [countRes, tasksRes, depsRes] = await Promise.all([
            fetch(`/tasks/count?${countParams}`),
            fetch(`/tasks?${filterParams}`),
            fetch(`/task_deps?limit=500`)
        ]);
        
        if (!tasksRes.ok) {
            contentDiv.innerHTML = `<p class="error">Erreur ${tasksRes.status}</p>`;
            return;
        }
        
        // Mettre à jour le nombre total
        if (countRes.ok) {
            const countData = await countRes.json();
            pagination.setTotalCount(countData.count || 0);
        }
        
        tasks = await tasksRes.json();

        // Charger les dépendances
        if (depsRes.ok) {
            taskDeps = await depsRes.json();
        } else {
            taskDeps = [];
        }

        // Mettre à jour la pagination
        const paginationContainer = document.getElementById('pagination-container');
        if (paginationContainer) {
            paginationContainer.innerHTML = '';
            const paginationEl = pagination.render(paginationContainer);
            if (paginationEl.updateDisplay) {
                paginationEl.updateDisplay();
            }
        }

        // Afficher les tâches (filtrage recherche côté client temporaire — ADR-0003)
        if (!Array.isArray(tasks) || tasks.length === 0) {
            contentDiv.innerHTML = '<p class="muted">Aucune tâche trouvée.</p>';
            return;
        }

        await renderTasksInContent(tasks);
    } catch (e) {
        contentDiv.innerHTML = `<p class="error">${e.message}</p>`;
    }
}

/**
 * Filtre les tâches par recherche (titre, description, ID) — temporaire côté client (ADR-0003).
 * @param {Array<{ id?: string, title?: string, description?: string }>} taskList
 * @param {string} query
 * @returns {typeof taskList}
 */
function filterTasksBySearch(taskList, query) {
    if (!query) return taskList;
    const q = query.toLowerCase();
    return taskList.filter((t) => {
        const title = (t.title || '').toLowerCase();
        const desc = (t.description || '').toLowerCase();
        const id = (t.id || '').toLowerCase();
        return title.includes(q) || desc.includes(q) || id.includes(q);
    });
}

/**
 * Trie les tâches par colonne et ordre (côté client).
 * @param {Array<Record<string, unknown>>} taskList
 * @param {string} by - title | status | role | phase | milestone | updated_at
 * @param {string} order - asc | desc
 * @returns {typeof taskList}
 */
function sortTasks(taskList, by, order) {
    const mult = order === 'asc' ? 1 : -1;
    const key = by === 'phase' ? 'phase_id' : by === 'milestone' ? 'milestone_id' : by;
    return [...taskList].sort((a, b) => {
        let va = a[key];
        let vb = b[key];
        if (by === 'updated_at' || by === 'title') {
            va = va != null ? String(va) : '';
            vb = vb != null ? String(vb) : '';
        } else {
            va = va != null ? String(va) : '';
            vb = vb != null ? String(vb) : '';
        }
        const cmp = va.localeCompare(vb, undefined, { numeric: by === 'updated_at' });
        return mult * cmp;
    });
}

/**
 * Groupe les tâches par clé (phase_id, milestone_id, status, role). Retourne Map<string, tâches[]>.
 * @param {Array<Record<string, unknown>>} taskList
 * @param {string} groupKey - phase | milestone | status | role
 * @returns {Map<string, typeof taskList>}
 */
function groupTasksBy(taskList, groupKey) {
    const keyField = groupKey === 'phase' ? 'phase_id' : groupKey === 'milestone' ? 'milestone_id' : groupKey;
    const map = new Map();
    for (const t of taskList) {
        const k = (t[keyField] != null ? String(t[keyField]) : '') || '—';
        if (!map.has(k)) map.set(k, []);
        map.get(k).push(t);
    }
    return map;
}

/**
 * Rendu des tâches dans #tasks-content : filtre recherche, tri, groupement, tableau (avec colonnes triables et sections repliables).
 * @param {Array<Record<string, unknown>>} taskList - Tâches brutes (page courante)
 */
async function renderTasksInContent(taskList) {
    const contentDiv = document.getElementById('tasks-content');
    if (!contentDiv) return;

    const filtered = filterTasksBySearch(taskList, searchQuery);
    const sorted = sortTasks(filtered, sortBy, sortOrder);

    const idToStatus = new Map(taskList.map((t) => [t.id, t.status]));
    const taskIds = new Set(idToStatus.keys());
    const depsForOurTasks = taskDeps.filter((d) => taskIds.has(d.task_id));
    const missingDepIds = [...new Set(depsForOurTasks.map((d) => d.depends_on).filter((id) => !idToStatus.has(id)))];
    await Promise.all(missingDepIds.map(async (id) => {
        try {
            const r = await fetch(`/task/${id}`);
            if (r.ok) {
                const j = await r.json();
                idToStatus.set(id, j.status);
            }
        } catch (_) {}
    }));
    const isBlocked = (t) => {
        const deps = depsForOurTasks.filter((d) => d.task_id === t.id);
        return deps.some((d) => (idToStatus.get(d.depends_on) || '') !== 'done');
    };

    const opts = { onTaskClick: (t) => loadTask(t.id), getStatusSuffix: isBlocked };

    contentDiv.innerHTML = '';
    contentDiv.appendChild(el('h2', {}, 'Tâches'));
    if (searchQuery && taskList.length > 0) {
        contentDiv.appendChild(el('p', { class: 'muted list-search-hint' }, `Recherche « ${escapeHtml(searchQuery)} » (filtrage côté client — ADR-0003).`));
    }

    if (sorted.length === 0) {
        contentDiv.appendChild(el('p', { class: 'muted' }, 'Aucune tâche ne correspond aux critères.'));
        return;
    }

    if (groupBy) {
        const groups = groupTasksBy(sorted, groupBy);
        const statusLabels = { to_do: 'À faire', in_progress: 'En cours', done: 'Terminé' };
        const getGroupLabel = (key) => {
            if (key === '' || key === '—') return '—';
            if (groupBy === 'phase') return phases[key]?.name || key;
            if (groupBy === 'milestone') return milestones[key]?.name || key;
            if (groupBy === 'status') return statusLabels[key] || key;
            return key;
        };
        const sortedKeys = [...groups.keys()].sort((a, b) => getGroupLabel(a).localeCompare(getGroupLabel(b)));
        for (const key of sortedKeys) {
            const groupTasks = groups.get(key);
            const label = getGroupLabel(key);
            const section = el('div', { class: 'task-group-section' });
            const header = el('button', { type: 'button', class: 'task-group-header' });
            header.appendChild(el('span', { class: 'task-group-toggle' }, '▼'));
            header.appendChild(el('span', { class: 'task-group-label' }, escapeHtml(String(label))));
            header.appendChild(el('span', { class: 'task-group-count muted' }, ` (${groupTasks.length})`));
            const body = el('div', { class: 'task-group-body' });
            body.appendChild(createTaskListTable(groupTasks, { ...opts, sortable: true, currentSort: { by: sortBy, order: sortOrder }, onSort: (by, order) => { sortBy = by; sortOrder = order; updateListOptionsFromState(); syncListStateToUrl(); renderTasksInContent(tasks); } }));
            section.appendChild(header);
            section.appendChild(body);
            header.addEventListener('click', () => {
                body.classList.toggle('task-group-body--collapsed');
                header.querySelector('.task-group-toggle').textContent = body.classList.contains('task-group-body--collapsed') ? '▶' : '▼';
            });
            contentDiv.appendChild(section);
        }
    } else {
        const table = createTaskListTable(sorted, { ...opts, sortable: true, currentSort: { by: sortBy, order: sortOrder }, onSort: (by, order) => { sortBy = by; sortOrder = order; updateListOptionsFromState(); syncListStateToUrl(); renderTasksInContent(tasks); } });
        contentDiv.appendChild(table);
    }
}

function getMilestoneName(id) {
    const milestone = milestones[id] || null;
    if (!milestone) { return '—';}
    return milestone.id + (milestone.name ? " - " + milestone.name : '');
}
function getPhaseName(id) {
    const phase = phases[id] || null;
    if (!phase) { return '—';}
    return phase.id + (phase.name ? " - " + phase.name : '');
}

/**
 * Ferme le panneau peek (Étape 5 — ADR-0002). Si l'URL est #/task/:id, la remplace par l'état liste courant.
 */
function closePeek() {
    const peek = document.getElementById('app-peek');
    if (peek) {
        peek.classList.remove('app-peek--open');
        const inner = peek.querySelector('.app-peek-inner');
        if (inner) inner.innerHTML = '';
    }
    if (urlState.getViewFromUrl() === 'task') {
        syncListStateToUrl(true);
    }
}

/**
 * Ouvre le panneau latéral droit (peek) avec le détail de la tâche (Étape 5 — ADR-0002).
 * @param {string} taskId - ID de la tâche
 * @param {{ fromUrl?: boolean }} opts - fromUrl: true si ouverture depuis la route #/task/:id (deep link)
 */
async function openPeek(taskId, opts = {}) {
    const peek = document.getElementById('app-peek');
    const peekInner = peek?.querySelector('.app-peek-inner');
    if (!peek || !peekInner) return;

    peekInner.innerHTML = '<p class="muted">Chargement…</p>';
    peek.classList.add('app-peek--open');

    try {
        const [taskRes, depsRes, allDepsRes, notesRes] = await Promise.all([
            fetch(`/task/${taskId}`),
            fetch(`/task/${taskId}/deps`),
            fetch(`/task_deps?limit=500`),
            fetch(`/task/${taskId}/notes`)
        ]);
        if (!taskRes.ok) {
            peekInner.innerHTML = taskRes.status === 404 ? '<p class="error">Tâche introuvable.</p>' : `<p class="error">Erreur ${taskRes.status}</p>`;
            return;
        }
        const t = await taskRes.json();

        let deps = [];
        try { if (depsRes.ok) deps = await depsRes.json(); } catch (_) {}
        deps = Array.isArray(deps) ? deps : [];
        const parentIds = deps.map((d) => d.depends_on).filter(Boolean);

        let allDeps = [];
        try { if (allDepsRes.ok) allDeps = await allDepsRes.json(); } catch (_) {}
        allDeps = Array.isArray(allDeps) ? allDeps : [];
        const childIds = allDeps.filter((d) => d.depends_on === taskId).map((d) => d.task_id);

        let taskNotes = [];
        try { if (notesRes.ok) taskNotes = await notesRes.json(); } catch (_) {}
        taskNotes = Array.isArray(taskNotes) ? taskNotes : [];

        const parentTasks = [];
        const childTasks = [];
        await Promise.all([
            ...parentIds.map(async (pid) => {
                try {
                    const r = await fetch(`/task/${pid}`);
                    if (r.ok) parentTasks.push(await r.json());
                } catch (_) {}
            }),
            ...childIds.map(async (cid) => {
                try {
                    const r = await fetch(`/task/${cid}`);
                    if (r.ok) childTasks.push(await r.json());
                } catch (_) {}
            })
        ]);

        const isBlocked = parentTasks.length > 0 && parentTasks.some((p) => p && p.status !== 'done');

        const phaseId = t.phase_id || '';
        const milestoneId = t.milestone_id || '';
        const listPhase = phaseId ? `#/list?phase=${encodeURIComponent(phaseId)}` : '#/list';
        const listMilestone = milestoneId ? `${listPhase}&milestone=${encodeURIComponent(milestoneId)}` : listPhase;

        const breadcrumb = el('nav', { class: 'app-peek-breadcrumb', 'aria-label': 'Fil d\'Ariane' });
        if (phaseId) {
            const phaseName = (phases[phaseId]?.name ?? phaseId);
            breadcrumb.appendChild(el('a', { href: listPhase }, escapeHtml(phaseName)));
            breadcrumb.appendChild(el('span', { class: 'app-peek-breadcrumb-sep' }, ' › '));
        }
        if (milestoneId) {
            const milestoneName = (milestones[milestoneId]?.name ?? milestoneId);
            breadcrumb.appendChild(el('a', { href: listMilestone }, escapeHtml(milestoneName)));
            breadcrumb.appendChild(el('span', { class: 'app-peek-breadcrumb-sep' }, ' › '));
        }
        breadcrumb.appendChild(document.createTextNode(t.title || t.id || 'Sans titre'));

        const closeBtn = el('button', { type: 'button', class: 'app-peek-close', 'aria-label': 'Fermer le détail' }, '×');
        closeBtn.addEventListener('click', () => closePeek());

        const openInUrlBtn = el('button', { type: 'button', class: 'app-peek-open-url' }, 'Ouvrir dans l\'URL');
        openInUrlBtn.addEventListener('click', () => urlState.setTaskInUrl(taskId));

        const header = el('div', { class: 'app-peek-header' });
        header.appendChild(breadcrumb);
        const actions = el('div', { class: 'app-peek-actions' });
        actions.appendChild(openInUrlBtn);
        actions.appendChild(closeBtn);
        header.appendChild(actions);

        const content = el('div', { class: 'app-peek-content' });
        const div = el('div', { class: 'task-detail' });
        div.appendChild(el('h2', {}, escapeHtml(t.title || 'Sans titre')));
        div.appendChild(el('p', { class: 'task-description' }, escapeHtml(t.description || '')));

        const phaseLabel = getPhaseName(t.phase_id);
        const milestoneLabel = getMilestoneName(t.milestone_id);
        const meta = document.createElement('table');
        meta.className = 'task-meta';
        const rows = [
            ['ID', String(t.id || '—'), 'uuid monospace'],
            ['Statut', null, ''],
            ['Rôle', String(t.role || '—'), (t.role || '') ? '' : 'error'],
            ['Phase', String(phaseLabel), ''],
            ['Jalon', String(milestoneLabel), ''],
            ['Ordre', t.sort_order != null ? String(t.sort_order) : '—', ''],
            ['Créé le', t.created_at != null ? String(t.created_at) : '—', ''],
            ['Créé par', String(t.creator || '—'), ''],
            ['Mis à jour le', t.updated_at != null ? String(t.updated_at) : '—', '']
        ];
        for (const [label, val, className] of rows) {
            const labelTd = el('td', { class: 'task-meta-label' }, label);
            let valueTd;
            if (label === 'Statut') {
                valueTd = el('td', {}, t.status || '—');
                if (isBlocked) valueTd.appendChild(el('span', { class: 'blocked' }, ' (blocked)'));
            } else {
                valueTd = el('td', { class: className }, escapeHtml(val));
            }
            meta.appendChild(el('tr', {}, labelTd, valueTd));
        }
        div.appendChild(el('div', { class: 'card' }, meta));

        div.appendChild(el('h3', {}, 'Historique des notes'));
        if (taskNotes.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune note pour cette tâche.'));
        } else {
            const notesList = document.createElement('div');
            notesList.className = 'task-notes-history';
            for (const n of taskNotes) {
                const noteCard = document.createElement('div');
                noteCard.className = 'task-note-card';
                const metaLine = [];
                if (n.kind) metaLine.push(escapeHtml(n.kind));
                if (n.role) metaLine.push(escapeHtml(n.role));
                if (n.created_at) metaLine.push(escapeHtml(n.created_at));
                const metaStr = metaLine.length ? metaLine.join(' · ') : '';
                noteCard.appendChild(el('div', { class: 'task-note-meta muted' }, metaStr));
                noteCard.appendChild(el('div', { class: 'task-note-content' }, escapeHtml(n.content || '')));
                notesList.appendChild(noteCard);
            }
            div.appendChild(notesList);
        }

        div.appendChild(el('h3', {}, 'Tâches parentes'));
        if (parentTasks.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune tâche.'));
        } else {
            div.appendChild(createTaskListTable(parentTasks, { onTaskClick: (task) => openPeek(task.id) }));
        }
        div.appendChild(el('h3', {}, 'Tâches enfants'));
        if (childTasks.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune tâche.'));
        } else {
            div.appendChild(createTaskListTable(childTasks, { onTaskClick: (task) => openPeek(task.id) }));
        }

        content.appendChild(div);
        peekInner.innerHTML = '';
        peekInner.appendChild(header);
        peekInner.appendChild(content);
    } catch (e) {
        peekInner.innerHTML = `<p class="error">${e.message}</p>`;
    }
}

/**
 * Charge les détails d'une tâche : ouvre le panneau peek (Étape 5 — ADR-0002).
 */
function loadTask(id) {
    openPeek(id);
}

let escapeHTMLElement = null;
function escapeHtml(s) {
    if (escapeHTMLElement === null) {
        escapeHTMLElement = document.createElement('div');
    }
    escapeHTMLElement.textContent = s;
    return escapeHTMLElement.innerHTML;
}

/** Colonnes du tableau liste (clé API / affichage) */
const LIST_TABLE_COLUMNS = [
    { key: 'id', label: 'id' },
    { key: 'title', label: 'title' },
    { key: 'role', label: 'role' },
    { key: 'status', label: 'status' },
    { key: 'milestone', label: 'milestone' },
    { key: 'phase', label: 'phase' },
    { key: 'updated_at', label: 'changed' }
];

/**
 * Tableau liste de tâches réutilisable (tri optionnel, colonnes cliquables).
 * @param {Array<Record<string, unknown>>} tasks
 * @param {{ onTaskClick: (t: Record<string, unknown>) => void, getStatusSuffix?: (t: Record<string, unknown>) => boolean, sortable?: boolean, currentSort?: { by: string, order: string }, onSort?: (by: string, order: string) => void }} opts
 */
function createTaskListTable(tasks, { onTaskClick, getStatusSuffix = () => false, sortable = false, currentSort = {}, onSort = null }) {
    const table = document.createElement('table');
    table.className = 'tasks-table';
    const thead = document.createElement('thead');
    const headerCells = LIST_TABLE_COLUMNS.map((col) => {
        const th = document.createElement('th');
        th.className = 'tasks-table-th';
        if (sortable && (col.key === 'title' || col.key === 'status' || col.key === 'role' || col.key === 'phase' || col.key === 'milestone' || col.key === 'updated_at')) {
            const sortKey = col.key === 'phase' ? 'phase_id' : col.key === 'milestone' ? 'milestone_id' : col.key;
            const isActive = currentSort.by === sortKey;
            const nextOrder = isActive && currentSort.order === 'asc' ? 'desc' : 'asc';
            th.appendChild(el('button', { type: 'button', class: `tasks-table-sort ${isActive ? 'tasks-table-sort--active' : ''}` }, col.label, isActive ? (currentSort.order === 'asc' ? ' ↑' : ' ↓') : ''));
            th.addEventListener('click', () => onSort && onSort(sortKey, nextOrder));
        } else {
            th.textContent = col.label;
        }
        return th;
    });
    thead.appendChild(el('tr', {}, ...headerCells));
    table.appendChild(thead);
    const tbody = document.createElement('tbody');
    for (const t of tasks) {
        const suffix = getStatusSuffix(t);

        let label = t.status || '';
        if (label === 'to_do' && suffix) {
            label = 'blocked';
        }

        let updated = new Date(t.updated_at).toLocaleString();
        if (updated === 'Invalid Date') updated = '';
        let timeAgo = new Date(t.updated_at).getTime() - new Date().getTime();
        if (timeAgo < 0) timeAgo = 0;
        timeAgo = Math.floor(timeAgo / 1000 / 3600 / 24);
        timeAgo = `${timeAgo} day${timeAgo !== 1 ? 's' : ''} ago`;
        

        const milestoneName = (milestones[t.milestone_id]?.name ?? t.milestone_id ?? '');
        const phaseName = (phases[t.phase_id]?.name ?? t.phase_id ?? '');
        const tr = el('tr', { 'data-task-id': t.id , class: label },
            el('td', { class: 'monospace uuid' }, escapeHtml(String(t.id ?? '').substring(0, 8))),
            el('td', {}, el('a', { href: '#' }, escapeHtml(t.title || t.id || 'untitled'))),
            el('td', {}, escapeHtml(t.role || '')),
            el('td', {},
                el('span', { class: label }, label)
            ),
            el('td', {}, escapeHtml(milestoneName)),
            el('td', {}, escapeHtml(phaseName)),
            el('td', {class: 'muted'}, escapeHtml(timeAgo))
        );
        tr.addEventListener('click', (e) => { e.preventDefault(); onTaskClick(t); });
        tbody.appendChild(tr);
    }
    table.appendChild(tbody);
    return table;
}

// Démarrer l'application
init();
