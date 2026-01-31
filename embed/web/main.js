import { el } from './dom.js';
import { Filters } from './filters.js';
import { Pagination } from './pagination.js';

const app = document.getElementById('app');

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
 * Affiche la vue principale avec filtres et pagination
 */
function renderMainView() {
    const ALL = ''
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
    const phaseOptions = [].concat(
        Object.values(phases).map(p => ({ value: p.id, label: `${p.id}: ${p.name || p.id}` }))
    );
    filters.updateSelectOptions('phase', phaseOptions);
    
    const milestoneOptions = [].concat(
        Object.values(milestones).map(m => ({ value: m.id, label: `${m.id}: ${m.name || m.id}` }))
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
    
    if (Object.keys(phases).length === 0) {
        container.appendChild(el('p', { class: 'muted' }, 'Aucune phase.'));
        return;
    }
    
    const phasesGrid = document.createElement('div');
    phasesGrid.className = 'phases-grid';
    
    for (const phase of Object.values(phases)) {
        const phaseCard = document.createElement('div');
        phaseCard.className = `phase-card phase-${phase.status || 'to_do'}`;
        
        const phaseHeader = document.createElement('div');
        phaseHeader.className = 'phase-header';
        phaseHeader.appendChild(el('h3', {}, escapeHtml(phase.name || phase.id)));
        phaseHeader.appendChild(el('span', { class: `status-badge status-${phase.status || 'to_do'}` }, phase.status || 'to_do'));
        phaseCard.appendChild(phaseHeader);
        
        // Compter les milestones et tâches pour cette phase
        const phaseMilestones = Object.values(milestones).filter(m => m.phase_id === phase.id);
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
            getStatusSuffix: (t) => isBlocked(t)
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
 * Charge les détails d'une tâche
 */
async function loadTask(id) {
    app.innerHTML = '';
    try {
        const [taskRes, depsRes, allDepsRes, notesRes] = await Promise.all([
            fetch(`/task/${id}`),
            fetch(`/task/${id}/deps`),
            fetch(`/task_deps?limit=500`),
            fetch(`/task/${id}/notes`)
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
