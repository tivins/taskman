const app = document.getElementById('app');

async function loadTasks(limit = 20, page = 1) {
  app.innerHTML = '';
  try {
    const r = await fetch(`/tasks?limit=${limit}&page=${page}`);
    if (!r.ok) { app.innerHTML = `<p class="error">Erreur ${r.status}</p>`; return; }
    const data = await r.json();
    if (!Array.isArray(data) || data.length === 0) {
      app.innerHTML = '<p>Aucune tâche.</p>';
      return;
    }
    const ul = document.createElement('ul');
    for (const t of data) {
      const li = document.createElement('li');
      const a = document.createElement('a');
      a.href = '#'; 
      a.textContent = escapeHtml(t.title || t.id || 'sans titre') + ' (' + escapeHtml(t.role || '') + ')';
      a.dataset.id = t.id;
      a.addEventListener('click', (e) => { e.preventDefault(); loadTask(t.id); });
      li.appendChild(a);
      ul.appendChild(li);
    }
    app.appendChild(ul);
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
<p><a href="#" id="back">← Liste des tâches</a></p>`;
    div.querySelector('#back').addEventListener('click', (e) => { e.preventDefault(); loadTasks(); });
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

loadTasks();
