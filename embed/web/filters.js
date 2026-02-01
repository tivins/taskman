/**
 * Gestion des filtres pour les tâches (milestone, phase, role, status, blocked_filter).
 * Liste des rôles alignée sur le backend (src/util/roles.cpp) — source de vérité = backend.
 */
const allOpt = '— All —';

/** 14 rôles du backend (roles.cpp), ordre et valeurs identiques — exporté pour le dashboard */
export const ROLE_OPTIONS = [
    { value: 'project-manager', label: 'Project Manager' },
    { value: 'project-designer', label: 'Project Designer' },
    { value: 'software-architect', label: 'Software Architect' },
    { value: 'developer', label: 'Developer' },
    { value: 'summary-writer', label: 'Summary Writer' },
    { value: 'documentation-writer', label: 'Documentation Writer' },
    { value: 'art-director', label: 'Art Director' },
    { value: 'ui-designer', label: 'UI Designer' },
    { value: 'community-manager', label: 'Community Manager' },
    { value: 'ux-designer', label: 'UX Designer' },
    { value: 'qa-engineer', label: 'QA Engineer' },
    { value: 'devops-engineer', label: 'DevOps Engineer' },
    { value: 'product-owner', label: 'Product Owner' },
    { value: 'security-engineer', label: 'Security Engineer' }
];

export class Filters {
    constructor() {
        this.filters = {
            milestone: '',
            phase: '',
            role: '',
            status: '',
            blocked_filter: ''
        };
        this.onChangeCallback = null;
    }

    /**
     * Définit un callback appelé quand les filtres changent
     */
    setOnChange(callback) {
        this.onChangeCallback = callback;
    }

    /**
     * Met à jour un filtre
     */
    setFilter(name, value) {
        if (this.filters.hasOwnProperty(name)) {
            this.filters[name] = value || '';
            if (this.onChangeCallback) {
                this.onChangeCallback(this.filters);
            }
        }
    }

    /**
     * Récupère tous les filtres
     */
    getFilters() {
        return { ...this.filters };
    }

    /**
     * Construit les paramètres de requête à partir des filtres
     */
    buildQueryParams() {
        const params = new URLSearchParams();
        for (const [key, value] of Object.entries(this.filters)) {
            if (value) {
                params.append(key, value);
            }
        }
        return params;
    }

    /**
     * Crée l'interface utilisateur des filtres
     */
    render(container) {
        const filtersDiv = document.createElement('div');
        filtersDiv.className = 'filters-container';

        // Filtre Phase
        const phaseGroup = this.createSelectGroup('phase', 'Phase', []);
        filtersDiv.appendChild(phaseGroup);

        // Filtre Milestone
        const milestoneGroup = this.createSelectGroup('milestone', 'Milestone', []);
        filtersDiv.appendChild(milestoneGroup);

        // Filtre Role (14 rôles = reflet backend)
        const roleGroup = this.createSelectGroup('role', 'Rôle', [{ value: '', label: allOpt }, ...ROLE_OPTIONS]);
        filtersDiv.appendChild(roleGroup);

        // Filtre Status
        const statusGroup = this.createSelectGroup('status', 'Statut', [
            { value: '', label: allOpt },
            { value: 'to_do', label: 'À faire' },
            { value: 'in_progress', label: 'En cours' },
            { value: 'done', label: 'Terminé' }
        ]);
        filtersDiv.appendChild(statusGroup);

        // Filtre Blocked (API: blocked_filter=blocked|unblocked)
        const blockedGroup = this.createSelectGroup('blocked_filter', 'Blocage', [
            { value: '', label: allOpt },
            { value: 'blocked', label: 'Blockées' },
            { value: 'unblocked', label: 'Non blockées' }
        ]);
        filtersDiv.appendChild(blockedGroup);

        // Bouton réinitialiser
        const resetBtn = document.createElement('button');
        resetBtn.className = 'filter-reset';
        resetBtn.textContent = 'Réinitialiser';
        resetBtn.type = 'button';
        resetBtn.addEventListener('click', () => {
            this.filters = {
                milestone: '',
                phase: '',
                role: '',
                status: '',
                blocked_filter: ''
            };
            filtersDiv.querySelectorAll('select').forEach(select => {
                select.value = '';
            });
            if (this.onChangeCallback) {
                this.onChangeCallback(this.filters);
            }
        });
        filtersDiv.appendChild(resetBtn);

        container.appendChild(filtersDiv);
        return filtersDiv;
    }

    /**
     * Crée un groupe de sélection (label + select)
     */
    createSelectGroup(name, label, options) {
        const group = document.createElement('div');
        group.className = 'filter-group';

        const labelEl = document.createElement('label');
        labelEl.textContent = label;
        labelEl.setAttribute('for', `filter-${name}`);
        group.appendChild(labelEl);

        const select = document.createElement('select');
        select.id = `filter-${name}`;
        select.className = 'filter-select';

        // Options par défaut si non fournies
        if (options.length === 0) {
            options.push({ value: '', label: allOpt });
        }

        options.forEach(opt => {
            const option = document.createElement('option');
            option.value = opt.value;
            option.textContent = opt.label;
            if (this.filters[name] === opt.value) {
                option.selected = true;
            }
            select.appendChild(option);
        });

        select.addEventListener('change', (e) => {
            this.setFilter(name, e.target.value);
        });

        group.appendChild(select);
        return group;
    }

    /**
     * Met à jour les options d'un select (pour phases et milestones dynamiques)
     */
    updateSelectOptions(name, options) {
        const select = document.getElementById(`filter-${name}`);
        if (!select) return;

        const currentValue = select.value;
        select.innerHTML = '';
        
        const defaultOpt = document.createElement('option');
        defaultOpt.value = '';
        defaultOpt.textContent = allOpt;
        select.appendChild(defaultOpt);

        options.forEach(opt => {
            const option = document.createElement('option');
            option.value = opt.value;
            option.textContent = opt.label;
            select.appendChild(option);
        });

        // Restaurer la valeur si elle existe toujours
        if (currentValue && options.some(opt => opt.value === currentValue)) {
            select.value = currentValue;
        } else {
            select.value = '';
            this.setFilter(name, '');
        }
    }
}
