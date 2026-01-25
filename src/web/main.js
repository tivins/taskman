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

        const table = document.createElement('table');
        table.className = 'tasks-table';
        const thead = document.createElement('thead');
        const headerRow = document.createElement('tr');
        ['id', 'title', 'role', 'status', 'milestone', 'phase'].forEach((h) => {
            const th = document.createElement('th');
            th.textContent = h;
            headerRow.appendChild(th);
        });
        thead.appendChild(headerRow);
        table.appendChild(thead);

        const tbody = document.createElement('tbody');
        for (const t of data) {
            const tr = el('tr', { 'data-task-id': t.id },
                el('td', { class: 'monospace' }, escapeHtml(String(t.id ?? '').substring(0, 8))),
                el('td', {}, el('a', { href: '#' }, escapeHtml(t.title || t.id || 'untitled'))),
                el('td', {}, escapeHtml(t.role || '')),
                el('td', {}, 
                  el('span', { class: t.status === 'done' ? 'done' : t.status === 'in_progress' ? 'in-progress' : t.status === 'to_do' ? 'to-do' : '' }, t.status), 
                  el('span', { class: isBlocked(t) ? 'blocked' : '' }, isBlocked(t) ? '(blocked)' : '')
                ),
                el('td', {}, escapeHtml(t.milestone_id || '')),
                el('td', {}, escapeHtml(t.phase_id || ''))
            );
            tr.addEventListener('click', (e) => {
                e.preventDefault();
                loadTask(t.id);
            });
            tbody.appendChild(tr);
        }
        table.appendChild(tbody);
        app.appendChild(table);
    } catch (e) {
        app.innerHTML = `<p class="error">${e.message}</p>`;
    }
}

async function loadTask(id) {
    app.innerHTML = '';
    try {
        const r = await fetch(`/task/${id}`);
        if (!r.ok) {
            app.innerHTML = r.status === 404 ? '<p class="error">Tâche introuvable.</p>' : `<p class="error">Erreur ${r.status}</p>`;
            return;
        }
        const t = await r.json();
        const div = document.createElement('div');
        div.className = 'task-detail';
        div.innerHTML = `<p><strong>${escapeHtml(t.title || '')}</strong></p>
<p>${escapeHtml(t.description || '')}</p>
<p>Statut: ${escapeHtml(t.status || '')} | Rôle: ${escapeHtml(t.role || '')}</p>
<p><a href="#" id="back">← tasks list</a></p>`;
        div.querySelector('#back').addEventListener('click', (e) => {
            e.preventDefault();
            loadTasks();
        });
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
        elem.setAttribute(key, value);
    }
    for (const child of children) {
        if (typeof child === 'string') {
            elem.appendChild(document.createTextNode(child));
        } else {
            elem.appendChild(child);
        }
    }
    return elem;
}

loadTasks();
