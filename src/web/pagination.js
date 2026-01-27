/**
 * Gestion de la pagination pour les tâches
 */

import { el } from './dom.js';

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
        const info = this.getInfo();
        const infoSpan = el('span', { class: 'pagination-info' });
        const pageSpan = el('span', { class: 'pagination-page' }, `${this.currentPage} / ${info.totalPages}`);

        const firstBtn = el('button', { class: 'pagination-btn' }, '«');
        firstBtn.disabled = this.currentPage === 1;
        firstBtn.addEventListener('click', () => this.goToPage(1));

        const prevBtn = el('button', { class: 'pagination-btn' }, '‹');
        prevBtn.disabled = this.currentPage === 1;
        prevBtn.addEventListener('click', () => this.previousPage());

        const nextBtn = el('button', { class: 'pagination-btn' }, '›');
        nextBtn.disabled = this.currentPage >= info.totalPages;
        nextBtn.addEventListener('click', () => this.nextPage());

        const lastBtn = el('button', { class: 'pagination-btn' }, '»');
        lastBtn.disabled = this.currentPage >= info.totalPages;
        lastBtn.addEventListener('click', () => this.goToPage(info.totalPages));

        const controlsDiv = el('div', { class: 'pagination-controls' },
            firstBtn, prevBtn, pageSpan, infoSpan, nextBtn, lastBtn
        );
        const paginationDiv = el('div', { class: 'pagination-container' }, controlsDiv);

        // Fonction pour mettre à jour l'affichage
        const updateDisplay = () => {
            const newInfo = this.getInfo();
            infoSpan.textContent = `(${newInfo.start}-${newInfo.end} / ${newInfo.total})`;
            pageSpan.textContent = `${newInfo.currentPage} / ${newInfo.totalPages}`;
            firstBtn.disabled = newInfo.currentPage === 1;
            prevBtn.disabled = newInfo.currentPage === 1;
            nextBtn.disabled = newInfo.currentPage >= newInfo.totalPages;
            lastBtn.disabled = newInfo.currentPage >= newInfo.totalPages;
        };

        paginationDiv.updateDisplay = updateDisplay;
        updateDisplay();

        container.appendChild(paginationDiv);
        return paginationDiv;
    }
}
