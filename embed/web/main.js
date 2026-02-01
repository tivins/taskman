import { el } from './dom.js';
import { Filters, ROLE_OPTIONS } from './filters.js';
import { Pagination } from './pagination.js';

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

/**
 * Initialisation de l'application
 */
async function init() {
    // Charger les phases et milestones pour les filtres
    await loadPhasesAndMilestones();
    updateSidebarTree();

    // Configurer les callbacks
    filters.setOnChange(() => {
        pagination.goToPage(1);
        loadTasks();
    });
    
    pagination.setOnPageChange(() => {
        loadTasks();
    });

    // Shell (header fixe + zone principale) puis vue liste
    ensureAppShell();
    setActiveNav('tasks');
    renderMainView();
    await loadTasks();
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
    nav.appendChild(el('a', { href: '#', class: 'app-nav-link app-nav-link-active', 'data-view': 'tasks' }, 'Tâches'));
    nav.appendChild(el('a', { href: '#overview', class: 'app-nav-link', 'data-view': 'overview' }, 'Vue d\'ensemble'));
    header.appendChild(nav);
    const headerActions = el('div', { class: 'app-header-actions' });
    header.appendChild(headerActions);
    shell.appendChild(header);

    const body = el('div', { class: 'app-body' });
    const sidebar = createSidebar();
    body.appendChild(sidebar);

    const main = el('div', { class: 'app-main' });
    body.appendChild(main);
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
                renderMainView();
                loadTasks();
            } else if (view === 'overview') {
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
    quickLinks.appendChild(el('a', { href: '#', class: 'app-sidebar-link', 'data-view': 'tasks' }, 'Tâches'));
    quickLinks.appendChild(el('a', { href: '#overview', class: 'app-sidebar-link', 'data-view': 'overview' }, 'Vue d\'ensemble'));
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
                renderMainView();
                loadTasks();
            } else if (view === 'overview') {
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
 * Affiche la vue principale (liste) avec filtres et pagination dans la zone principale du shell.
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
            // Mettre à jour l'affichage de la pagination
            if (paginationEl.updateDisplay) {
                paginationEl.updateDisplay();
            }
        }
        
        // Afficher les tâches
        if (!Array.isArray(tasks) || tasks.length === 0) {
            contentDiv.innerHTML = '<p class="muted">Aucune tâche trouvée.</p>';
            return;
        }
        
        // Charger les statuts des dépendances manquantes
        const idToStatus = new Map(tasks.map(t => [t.id, t.status]));
        const taskIds = new Set(idToStatus.keys());
        const depsForOurTasks = taskDeps.filter(d => taskIds.has(d.task_id));
        const missingDepIds = [...new Set(depsForOurTasks.map(d => d.depends_on).filter(id => !idToStatus.has(id)))];
        
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
            const taskDeps = depsForOurTasks.filter(d => d.task_id === t.id);
            return taskDeps.some(d => (idToStatus.get(d.depends_on) || '') !== 'done');
        };
        
        const table = createTaskListTable(tasks, {
            onTaskClick: (t) => loadTask(t.id),
            getStatusSuffix: (t) => isBlocked(t)
        });
        
        contentDiv.innerHTML = '';
        contentDiv.appendChild(el('h2', {}, 'Tâches'));
        contentDiv.appendChild(table);
        
    } catch (e) {
        contentDiv.innerHTML = `<p class="error">${e.message}</p>`;
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
 * Charge les détails d'une tâche dans la zone principale (shell conservé).
 */
async function loadTask(id) {
    const main = appMain || app.querySelector('.app-main');
    if (!main) return;
    main.innerHTML = '';
    try {
        const [taskRes, depsRes, allDepsRes, notesRes] = await Promise.all([
            fetch(`/task/${id}`),
            fetch(`/task/${id}/deps`),
            fetch(`/task_deps?limit=500`),
            fetch(`/task/${id}/notes`)
        ]);
        if (!taskRes.ok) {
            main.innerHTML = taskRes.status === 404 ? '<p class="error">Tâche introuvable.</p>' : `<p class="error">Erreur ${taskRes.status}</p>`;
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
        const childIds = allDeps.filter((d) => d.depends_on === id).map((d) => d.task_id);

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

        const div = document.createElement('div');
        div.className = 'task-detail';

        const back = el('p', {}, el('a', { href: '#', id: 'back' , class: 'btn ghost'}, '← Back'));
        back.querySelector('#back').addEventListener('click', (e) => { e.preventDefault(); init(); });
        div.appendChild(back);

        const head = el('div', {class:"", style:"max-width:40em;margin:3rem;font-size:125%"},
            el('h2', {}, escapeHtml(t.title || 'Sans titre')),
            el('p', { class: 'task-description' }, escapeHtml(t.description))
        );
        div.appendChild(head);

        const isBlocked = parentTasks.length > 0 && parentTasks.some((p) => p && p.status !== 'done');
        const meta = document.createElement('table');
        meta.className = 'task-meta';
        const phaseLabel = getPhaseName(t.phase_id);
        const milestoneLabel = getMilestoneName(t.milestone_id);
        const rows = [
            // ['Subject', escapeHtml(t.title || 'Sans titre'), 'row-title'],
            // ['Description', escapeHtml(t.description || 'Everything is in the title.'), 'row-description'],
            ['ID', String(t.id || '—'), 'uuid monospace'],
            ['Statut', null, ''],
            ['Rôle', String(t.role || '— THIS TASK IS NOT ASSIGNED —'), (t.role || '' ) ? '' : 'error'],
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
            }
            // else if (label === 'Subject') {
            //     valueTd = el('td', {}, el('div',{style:'width:40em;font-size:150%'},val));
            // }
            // else if (label === 'Description') {
            //     valueTd = el('td', {}, el('div',{style:''},val));
            // }
            else {
                valueTd = el('td', {class:className}, escapeHtml(val));
            }
            meta.appendChild(el('tr', {}, labelTd, valueTd));
        }


        div.appendChild(el('div', {class: 'card'}, meta));

        div.appendChild(el('h3', {style:"margin:2rem 0 0;"}, 'Historique des notes'));
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

        div.appendChild(el('h3', {style:"margin:2rem 0 0;"}, 'Tâches parentes (dont dépend cette tâche)'));
        if (parentTasks.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune tâche.'));
        } else {
            div.appendChild(createTaskListTable(parentTasks, { onTaskClick: (task) => loadTask(task.id) }));
        }

        div.appendChild(el('h3', {style:"margin:2rem 0 0;"}, 'Tâches enfants (qui dépendent de cette tâche)'));
        if (childTasks.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune tâche.'));
        } else {
            div.appendChild(createTaskListTable(childTasks, { onTaskClick: (task) => loadTask(task.id) }));
        }

        main.appendChild(div);
    } catch (e) {
        main.innerHTML = `<p class="error">${e.message}</p>`;
    }
}

let escapeHTMLElement = null;
function escapeHtml(s) {
    if (escapeHTMLElement === null) {
        escapeHTMLElement = document.createElement('div');
    }
    escapeHTMLElement.textContent = s;
    return escapeHTMLElement.innerHTML;
}

/**
 * Reusabel task list
 */
function createTaskListTable(tasks, { onTaskClick, getStatusSuffix = () => false }) {
    const table = document.createElement('table');
    table.className = 'tasks-table';
    const thead = document.createElement('thead');
    thead.appendChild(el('tr', {}, ...['id', 'title', 'role', 'status', 'milestone', 'phase', 'changed'].map((h) => {
        const th = document.createElement('th');
        th.textContent = h;
        return th;
    })));
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
