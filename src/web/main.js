const app = document.getElementById('app');

async function loadTasks(limit = 20, page = 1) {
    app.innerHTML = '';
    try {
        const r = await fetch(`/tasks?limit=${limit}&page=${page}`);
        if (!r.ok) {
            app.innerHTML = `<p class="error">Error ${r.status}</p>`;
            return;
        }
        const data = await r.json();
        if (!Array.isArray(data) || data.length === 0) {
            app.innerHTML = '<p>No task.</p>';
            return;
        }
        const grid = document.createElement('div');
        grid.className = 'tasks-grid';
        // En-têtes
        ['id', 'title', 'role', 'status', 'milestone', 'phase'].forEach((h) => {
            const th = document.createElement('div');
            th.className = 'tasks-th';
            th.textContent = h;
            grid.appendChild(th);
        });
        // Lignes
        for (const t of data) {
            const idCell = document.createElement('div');
            idCell.className = 'tasks-td monospace';
            idCell.textContent = escapeHtml(String(t.id ?? ''));
            grid.appendChild(idCell);

            const titleCell = document.createElement('div');
            titleCell.className = 'tasks-td';
            const a = document.createElement('a');
            a.href = '#';
            a.textContent = escapeHtml(t.title || t.id || 'untitled');
            a.addEventListener('click', (e) => {
                e.preventDefault();
                loadTask(t.id);
            });
            titleCell.appendChild(a);
            grid.appendChild(titleCell);

            const roleCell = document.createElement('div');
            roleCell.className = 'tasks-td';
            roleCell.textContent = escapeHtml(t.role || '');
            grid.appendChild(roleCell);

            const statusCell = document.createElement('div');
            statusCell.className = 'tasks-td';
            statusCell.textContent = escapeHtml(t.status || '');
            grid.appendChild(statusCell);

            const milestoneCell = document.createElement('div');
            milestoneCell.className = 'tasks-td';
            milestoneCell.textContent = escapeHtml(t.milestone_id || '');
            grid.appendChild(milestoneCell);

            const phaseCell = document.createElement('div');
            phaseCell.className = 'tasks-td';
            phaseCell.textContent = escapeHtml(t.phase_id || '');
            grid.appendChild(phaseCell);
        }
        app.appendChild(grid);
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

function escapeHtml(s) {
    const d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
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
