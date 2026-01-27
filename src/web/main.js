import { Filters } from './filters.js';
import { Pagination } from './pagination.js';

const app = document.getElementById('app');

// État global
let filters = new Filters();
let pagination = new Pagination();
let phases = [];
let milestones = [];
let tasks = [];
let taskDeps = [];

/**
 * Initialisation de l'application
 */
async function init() {
    // Charger les phases et milestones pour les filtres
    await loadPhasesAndMilestones();
    
    // Configurer les callbacks
    filters.setOnChange(() => {
        pagination.goToPage(1);
        loadTasks();
    });
    
    pagination.setOnPageChange(() => {
        loadTasks();
    });

    // Afficher la vue principale
    renderMainView();
    await loadTasks();
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
            phases = await phasesRes.json();
        }
        
        if (milestonesRes.ok) {
            milestones = await milestonesRes.json();
        }
    } catch (e) {
        console.error('Error loading phases/milestones:', e);
    }
}

/**
 * Affiche la vue principale avec filtres et pagination
 */
function renderMainView() {
    app.innerHTML = '';
    
    // En-tête avec vue d'ensemble
    // const header = el('div', { class: 'main-header' }, el('h1', {}, 'Gestionnaire de Projet'));
    const header = el('div', {});
    
    // Vue d'ensemble des phases
    /*
    const overviewDiv = document.createElement('div');
    overviewDiv.className = 'overview-container';
    overviewDiv.appendChild(el('h2', {}, 'Vue d\'ensemble'));
    const overviewContent = document.createElement('div');
    overviewContent.className = 'overview-content';
    overviewDiv.appendChild(overviewContent);
    header.appendChild(overviewDiv);
    */
    app.appendChild(header);
    
    // Filtres
    const filtersContainer = document.createElement('div');
    filtersContainer.className = 'filters-wrapper';
    filters.render(filtersContainer);
    app.appendChild(filtersContainer);
    
    // Mettre à jour les options des selects dynamiques
    const phaseOptions = [{ value: '', label: 'Tous' }].concat(
        phases.map(p => ({ value: p.id, label: `${p.id}: ${p.name || p.id}` }))
    );
    filters.updateSelectOptions('phase', phaseOptions);
    
    const milestoneOptions = [{ value: '', label: 'Tous' }].concat(
        milestones.map(m => ({ value: m.id, label: `${m.id}: ${m.name || m.id}` }))
    );
    filters.updateSelectOptions('milestone', milestoneOptions);
    
    // Zone de contenu pour les tâches
    const contentDiv = el('div', { class: 'tasks-content' }, 'tasks-content');
    contentDiv.className = 'tasks-content';
    contentDiv.id = 'tasks-content';
    app.appendChild(contentDiv);
    
    // Pagination
    const paginationContainer = document.createElement('div');
    paginationContainer.className = 'pagination-wrapper';
    paginationContainer.id = 'pagination-container';
    app.appendChild(paginationContainer);
    
    // Charger la vue d'ensemble
    // updateOverview(overviewContent);
}

/**
 * Met à jour la vue d'ensemble des phases
 */
async function updateOverview(container) {
    container.innerHTML = '';

    // Temporary disabled: overview is not used anymore
    return;
    
    if (phases.length === 0) {
        container.appendChild(el('p', { class: 'muted' }, 'Aucune phase.'));
        return;
    }
    
    const phasesGrid = document.createElement('div');
    phasesGrid.className = 'phases-grid';
    
    for (const phase of phases) {
        const phaseCard = document.createElement('div');
        phaseCard.className = `phase-card phase-${phase.status || 'to_do'}`;
        
        const phaseHeader = document.createElement('div');
        phaseHeader.className = 'phase-header';
        phaseHeader.appendChild(el('h3', {}, escapeHtml(phase.name || phase.id)));
        phaseHeader.appendChild(el('span', { class: `status-badge status-${phase.status || 'to_do'}` }, phase.status || 'to_do'));
        phaseCard.appendChild(phaseHeader);
        
        // Compter les milestones et tâches pour cette phase
        const phaseMilestones = milestones.filter(m => m.phase_id === phase.id);
        const phaseTasks = tasks.filter(t => t.phase_id === phase.id);
        const doneTasks = phaseTasks.filter(t => t.status === 'done').length;
        
        const stats = document.createElement('div');
        stats.className = 'phase-stats';
        stats.appendChild(el('div', {}, `Milestones: ${phaseMilestones.length}`));
        stats.appendChild(el('div', {}, `Tâches: ${doneTasks}/${phaseTasks.length}`));
        phaseCard.appendChild(stats);
        
        // Afficher les milestones
        if (phaseMilestones.length > 0) {
            const milestonesList = document.createElement('div');
            milestonesList.className = 'milestones-list';
            phaseMilestones.forEach(m => {
                const milestoneEl = document.createElement('div');
                milestoneEl.className = `milestone-item ${m.reached ? 'reached' : ''}`;
                milestoneEl.appendChild(el('span', {}, escapeHtml(m.name || m.id)));
                if (m.reached) {
                    milestoneEl.appendChild(el('span', { class: 'reached-badge' }, '✓'));
                }
                milestonesList.appendChild(milestoneEl);
            });
            phaseCard.appendChild(milestonesList);
        }
        
        phasesGrid.appendChild(phaseCard);
    }
    
    container.appendChild(phasesGrid);
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
            getStatusSuffix: (t) => isBlocked(t) ? 'blocked' : ''
        });
        
        contentDiv.innerHTML = '';
        contentDiv.appendChild(el('h2', {}, 'Tâches'));
        contentDiv.appendChild(table);
        
        // Mettre à jour la vue d'ensemble
        const overviewContent = document.querySelector('.overview-content');
        if (overviewContent) {
            updateOverview(overviewContent);
        }
        
    } catch (e) {
        contentDiv.innerHTML = `<p class="error">${e.message}</p>`;
    }
}

/**
 * Charge les détails d'une tâche
 */
async function loadTask(id) {
    app.innerHTML = '';
    try {
        const [taskRes, depsRes, allDepsRes] = await Promise.all([
            fetch(`/task/${id}`),
            fetch(`/task/${id}/deps`),
            fetch(`/task_deps?limit=500`)
        ]);
        if (!taskRes.ok) {
            app.innerHTML = taskRes.status === 404 ? '<p class="error">Tâche introuvable.</p>' : `<p class="error">Erreur ${taskRes.status}</p>`;
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

        const back = el('p', {}, el('a', { href: '#', id: 'back' }, '← liste des tâches'));
        back.querySelector('#back').addEventListener('click', (e) => { e.preventDefault(); init(); });
        div.appendChild(back);

        div.appendChild(el('h2', {}, escapeHtml(t.title || 'Sans titre')));
        if (t.description) div.appendChild(el('p', { class: 'task-description' }, escapeHtml(t.description)));

        const isBlocked = parentTasks.length > 0 && parentTasks.some((p) => p && p.status !== 'done');
        const meta = document.createElement('table');
        meta.className = 'task-meta';
        const rows = [
            ['ID', String(t.id || '—')],
            ['Statut', null],
            ['Rôle', String(t.role || '—')],
            ['Phase', String(t.phase_id || '—')],
            ['Jalon', String(t.milestone_id || '—')],
            ['Ordre', t.sort_order != null ? String(t.sort_order) : '—']
        ];
        for (const [label, val] of rows) {
            const labelTd = el('td', { class: 'task-meta-label' }, label);
            let valueTd;
            if (label === 'Statut') {
                valueTd = el('td', {}, t.status || '—');
                if (isBlocked) valueTd.appendChild(el('span', { class: 'blocked' }, ' (blocked)'));
            } else {
                valueTd = el('td', {}, escapeHtml(val));
            }
            meta.appendChild(el('tr', {}, labelTd, valueTd));
        }
        div.appendChild(meta);

        div.appendChild(el('h3', {}, 'Tâches parentes (dont dépend cette tâche)'));
        if (parentTasks.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune tâche.'));
        } else {
            div.appendChild(createTaskListTable(parentTasks, { onTaskClick: (task) => loadTask(task.id) }));
        }

        div.appendChild(el('h3', {}, 'Tâches enfants (qui dépendent de cette tâche)'));
        if (childTasks.length === 0) {
            div.appendChild(el('p', { class: 'muted' }, 'Aucune tâche.'));
        } else {
            div.appendChild(createTaskListTable(childTasks, { onTaskClick: (task) => loadTask(task.id) }));
        }

        app.appendChild(div);
    } catch (e) {
        app.innerHTML = `<p class="error">${e.message}</p>`;
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

function el(tag, attrs, ...children) {
    const elem = document.createElement(tag);
    for (const [key, value] of Object.entries(attrs)) {
        if (value != null) elem.setAttribute(key, value);
    }
    for (const child of children) {
        if (typeof child === 'string') {
            elem.appendChild(document.createTextNode(child));
        } else if (child != null) {
            elem.appendChild(child);
        }
    }
    return elem;
}

/**
 * Liste de tâches réutilisable
 */
function createTaskListTable(tasks, { onTaskClick, getStatusSuffix = () => '' }) {
    const table = document.createElement('table');
    table.className = 'tasks-table';
    const thead = document.createElement('thead');
    thead.appendChild(el('tr', {}, ...['id', 'title', 'role', 'status', 'milestone', 'phase'].map((h) => {
        const th = document.createElement('th');
        th.textContent = h;
        return th;
    })));
    table.appendChild(thead);
    const tbody = document.createElement('tbody');
    for (const t of tasks) {
        const suffix = getStatusSuffix(t);

        let label = t.status || '';
        if (label === 'to_do' && suffix === 'blocked') {
            label = 'blocked';
        }


        const tr = el('tr', { 'data-task-id': t.id },
            el('td', { class: 'monospace uuid' }, escapeHtml(String(t.id ?? '').substring(0, 8))),
            el('td', {}, el('a', { href: '#' }, escapeHtml(t.title || t.id || 'untitled'))),
            el('td', {}, escapeHtml(t.role || '')),
            el('td', {},
                el('span', { class: label }, label)
            ),
            el('td', {}, escapeHtml(t.milestone_id || '')),
            el('td', {}, escapeHtml(t.phase_id || ''))
        );
        tr.addEventListener('click', (e) => { e.preventDefault(); onTaskClick(t); });
        tbody.appendChild(tr);
    }
    table.appendChild(tbody);
    return table;
}

// Démarrer l'application
init();
