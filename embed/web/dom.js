/**
 * Utilitaire DOM : création d'éléments avec attributs et enfants
 *
 * @param {string} tag - Nom de l'élément HTML
 * @param {Object} attrs - Attributs (clé/valeur), null/undefined ignorés
 * @param {...(string|Node)} children - Enfants (texte ou nœuds DOM)
 * @returns {HTMLElement}
 */
export function el(tag, attrs, ...children) {
    const elem = document.createElement(tag);
    for (const [key, value] of Object.entries(attrs || {})) {
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
