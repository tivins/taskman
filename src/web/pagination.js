/**
 * Gestion de la pagination pour les tâches
 */

export class Pagination {
    constructor() {
        this.currentPage = 1;
        this.pageSize = 50;
        this.totalCount = 0;
        this.onPageChangeCallback = null;
    }

    /**
     * Définit un callback appelé quand la page change
     */
    setOnPageChange(callback) {
        this.onPageChangeCallback = callback;
    }

    /**
     * Définit la taille de page
     */
    setPageSize(size) {
        this.pageSize = Math.max(1, size);
        this.currentPage = 1; // Reset à la première page
    }

    /**
     * Définit le nombre total d'éléments
     */
    setTotalCount(count) {
        this.totalCount = Math.max(0, count);
        // Ajuster la page courante si nécessaire
        const maxPage = this.getTotalPages();
        if (this.currentPage > maxPage && maxPage > 0) {
            this.currentPage = maxPage;
        }
    }

    /**
     * Va à une page spécifique
     */
    goToPage(page) {
        const maxPage = this.getTotalPages();
        const newPage = Math.max(1, Math.min(page, maxPage));
        if (newPage !== this.currentPage) {
            this.currentPage = newPage;
            if (this.onPageChangeCallback) {
                this.onPageChangeCallback(this.currentPage);
            }
        }
    }

    /**
     * Page suivante
     */
    nextPage() {
        this.goToPage(this.currentPage + 1);
    }

    /**
     * Page précédente
     */
    previousPage() {
        this.goToPage(this.currentPage - 1);
    }

    /**
     * Calcule le nombre total de pages
     */
    getTotalPages() {
        return Math.max(1, Math.ceil(this.totalCount / this.pageSize));
    }

    /**
     * Récupère la page courante
     */
    getCurrentPage() {
        return this.currentPage;
    }

    /**
     * Récupère la taille de page
     */
    getPageSize() {
        return this.pageSize;
    }

    /**
     * Récupère l'offset pour la requête SQL
     */
    getOffset() {
        return (this.currentPage - 1) * this.pageSize;
    }

    /**
     * Récupère les informations de pagination pour l'affichage
     */
    getInfo() {
        const start = this.totalCount === 0 ? 0 : (this.currentPage - 1) * this.pageSize + 1;
        const end = Math.min(this.currentPage * this.pageSize, this.totalCount);
        return {
            start,
            end,
            total: this.totalCount,
            currentPage: this.currentPage,
            totalPages: this.getTotalPages()
        };
    }

    /**
     * Crée l'interface utilisateur de pagination
     */
    render(container) {
        const paginationDiv = document.createElement('div');
        paginationDiv.className = 'pagination-container';

        const info = this.getInfo();
        const infoSpan = document.createElement('span');
        infoSpan.className = 'pagination-info';
        paginationDiv.appendChild(infoSpan);

        const controlsDiv = document.createElement('div');
        controlsDiv.className = 'pagination-controls';

        // Bouton première page
        const firstBtn = document.createElement('button');
        firstBtn.className = 'pagination-btn';
        firstBtn.textContent = '«';
        firstBtn.disabled = this.currentPage === 1;
        firstBtn.addEventListener('click', () => this.goToPage(1));
        controlsDiv.appendChild(firstBtn);

        // Bouton page précédente
        const prevBtn = document.createElement('button');
        prevBtn.className = 'pagination-btn';
        prevBtn.textContent = '‹';
        prevBtn.disabled = this.currentPage === 1;
        prevBtn.addEventListener('click', () => this.previousPage());
        controlsDiv.appendChild(prevBtn);

        // Numéro de page
        const pageSpan = document.createElement('span');
        pageSpan.className = 'pagination-page';
        pageSpan.textContent = `${this.currentPage} / ${info.totalPages}`;
        controlsDiv.appendChild(pageSpan);

        // Bouton page suivante
        const nextBtn = document.createElement('button');
        nextBtn.className = 'pagination-btn';
        nextBtn.textContent = '›';
        nextBtn.disabled = this.currentPage >= info.totalPages;
        nextBtn.addEventListener('click', () => this.nextPage());
        controlsDiv.appendChild(nextBtn);

        // Bouton dernière page
        const lastBtn = document.createElement('button');
        lastBtn.className = 'pagination-btn';
        lastBtn.textContent = '»';
        lastBtn.disabled = this.currentPage >= info.totalPages;
        lastBtn.addEventListener('click', () => this.goToPage(info.totalPages));
        controlsDiv.appendChild(lastBtn);

        paginationDiv.appendChild(controlsDiv);

        // Fonction pour mettre à jour l'affichage
        const updateDisplay = () => {
            const newInfo = this.getInfo();
            infoSpan.textContent = `Affichage ${newInfo.start}-${newInfo.end} sur ${newInfo.total}`;
            pageSpan.textContent = `${newInfo.currentPage} / ${newInfo.totalPages}`;
            firstBtn.disabled = newInfo.currentPage === 1;
            prevBtn.disabled = newInfo.currentPage === 1;
            nextBtn.disabled = newInfo.currentPage >= newInfo.totalPages;
            lastBtn.disabled = newInfo.currentPage >= newInfo.totalPages;
        };

        // Stocker la fonction de mise à jour pour pouvoir la réutiliser
        paginationDiv.updateDisplay = updateDisplay;
        updateDisplay();

        container.appendChild(paginationDiv);
        return paginationDiv;
    }
}
