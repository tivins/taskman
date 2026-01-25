const app = document.getElementById('app');

async function loadTasks(limit = 20, page = 1) {
    app.innerHTML = '';
    try {
        const [tasksRes, depsRes] = await Promise.all([
            fetch(`/tasks?limit=${limit}&page=${page}`),
            fetch(`/task_deps?limit=500`)
        ]);
        if (!tasksRes.ok) {
            app.innerHTML = `<p class="error">Error ${tasksRes.status}</p>`;
            return;
        }
        const data = await tasksRes.json();
        if (!Array.isArray(data) || data.length === 0) {
            app.innerHTML = '<p>No task.</p>';
            return;
        }

        const idToStatus = new Map(data.map((t) => [t.id, t.status]));
        const taskIds = new Set(idToStatus.keys());
        let deps = [];
        try {
            const depsJson = depsRes.ok ? await depsRes.json() : [];
            deps = Array.isArray(depsJson) ? depsJson : [];
        } catch (_) {}
        const depsForOurTasks = deps.filter((d) => taskIds.has(d.task_id));
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
            const taskDeps = depsForOurTasks.filter((d) => d.task_id === t.id);
            return taskDeps.some((d) => (idToStatus.get(d.depends_on) || '') !== 'done');
        };

        const table = createTaskListTable(data, {
            onTaskClick: (t) => loadTask(t.id),
            getStatusSuffix: (t) => isBlocked(t) ? '(blocked)' : ''
        });
        app.appendChild(table);
    } catch (e) {
        app.innerHTML = `<p class="error">${e.message}</p>`;
    }
}

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
        back.querySelector('#back').addEventListener('click', (e) => { e.preventDefault(); loadTasks(); });
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
 * Liste de tâches réutilisable, même style que la vue liste (id, title, role, status, milestone, phase).
 * @param {Array} tasks - tableau de tâches { id, title, role, status, milestone_id, phase_id, ... }
 * @param {{ onTaskClick: (t) => void, getStatusSuffix?: (t) => string }} opts - onTaskClick requis; getStatusSuffix optionnel (ex: "(blocked)")
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
        const tr = el('tr', { 'data-task-id': t.id },
            el('td', { class: 'monospace' }, escapeHtml(String(t.id ?? '').substring(0, 8))),
            el('td', {}, el('a', { href: '#' }, escapeHtml(t.title || t.id || 'untitled'))),
            el('td', {}, escapeHtml(t.role || '')),
            el('td', {},
                el('span', { class: t.status === 'done' ? 'done' : t.status === 'in_progress' ? 'in-progress' : t.status === 'to_do' ? 'to-do' : '' }, t.status || ''),
                ...(suffix ? [el('span', { class: 'blocked' }, suffix)] : [])
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

loadTasks();
