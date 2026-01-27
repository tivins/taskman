/**
 * demo:generate implementation — creates a demo database.
 */

#include "demo.hpp"
#include "db.hpp"
#include "milestone.hpp"
#include "note.hpp"
#include "phase.hpp"
#include "task.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <uuid.h>

namespace {

std::string generate_uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uuids::uuid_random_generator gen(rng);
    uuids::uuid u = gen();
    return uuids::to_string(u);
}

} // namespace

namespace taskman {

int cmd_demo_generate(int argc, char* argv[], Database& db) {
    cxxopts::Options opts("taskman demo:generate", "Generate a demo database");
    opts.add_options()
        ("help", "Show help");

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << "taskman demo:generate\n\n"
                         "Create a database filled with a real-life example (e-commerce site MVP project).\n"
                         "The database path is determined by TASKMAN_DB_NAME environment variable\n"
                         "or defaults to 'project_tasks.db'.\n\n"
                         "If the database already exists, it will be removed and recreated.\n\n";
            return 0;
        }
    }

    try {
        opts.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    // Get database path
    const char* db_path_env = std::getenv("TASKMAN_DB_NAME");
    std::string db_path = (db_path_env && db_path_env[0] != '\0') ? db_path_env : "project_tasks.db";

    // Ensure database is closed before we try to delete it
    db.close();

    // Remove existing database and related files for a reproducible demo
    namespace fs = std::filesystem;
    if (fs::exists(db_path)) {
        try {
            fs::remove(db_path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "taskman: cannot remove " << db_path << " (" << e.what() << "). Close any process using it.\n";
            return 1;
        }
    }
    // Remove journal files
    for (const std::string& suffix : {"-journal", "-wal", "-shm"}) {
        std::string path = db_path + suffix;
        if (fs::exists(path)) {
            try {
                fs::remove(path);
            } catch (const fs::filesystem_error&) {
                // Ignore errors for journal files
            }
        }
    }

    std::cout << "Creating demo database with a real-life example (e-commerce site MVP project)...\n";

    // Reopen database
    if (!db.open(db_path.c_str())) {
        std::cerr << "taskman: failed to open database\n";
        return 1;
    }

    // Initialize schema
    if (!init_schema(db)) {
        std::cerr << "taskman: failed to initialize schema\n";
        return 1;
    }
    std::cout << "  init\n";

    // Phases
    if (!phase_add(db, "P1", "Design", "in_progress", 1)) return 1;
    if (!phase_add(db, "P2", "Development", "to_do", 2)) return 1;
    if (!phase_add(db, "P3", "Acceptance", "to_do", 3)) return 1;
    if (!phase_add(db, "P4", "Delivery", "to_do", 4)) return 1;
    std::cout << "  phases P1-P4\n";

    // Milestones
    if (!milestone_add(db, "M1", "P1", "Specs approved", "Document signed off by client", true)) return 1;
    if (!milestone_add(db, "M2", "P2", "MVP delivered", "Catalog, cart and test payment operational", false)) return 1;
    if (!milestone_add(db, "M3", "P3", "Acceptance OK", "E2E tests passing, blocking bugs resolved", false)) return 1;
    if (!milestone_add(db, "M4", "P4", "Production deployment", "App deployed and reachable", false)) return 1;
    std::cout << "  milestones M1-M4\n";

    // Tasks: quantities vary by phase/milestone; some unassigned, long descriptions, one long title for UI tests
    int order = 1;

    // --- P1 / M1 (Design — Specs approved): 6 tasks
    std::string t1 = generate_uuid_v4();
    if (!task_add(db, t1, "P1", "M1", "Write requirements document",
                  "Draft the functional and non-functional requirements document: MVP scope, personas, "
                  "priority use cases, technical constraints and expected deliverables. Include acceptance "
                  "criteria and business assumptions. Have the client approve before moving to development.",
                  "done", order++, "project-manager")) return 1;
    std::string t2 = generate_uuid_v4();
    if (!task_add(db, t2, "P1", "M1", "Specify auth API",
                  "Define the authentication API contract: endpoints (login, logout, refresh, revoke), "
                  "request/response schemas, error codes and security rules (HTTPS, tokens, expiration). "
                  "Document in OpenAPI and plan SSO/OAuth integration if needed.",
                  "done", order++, "software-architect")) return 1;
    std::string t3 = generate_uuid_v4();
    if (!task_add(db, t3, "P1", "M1", "Validate specs",
                  "Review product and technical specs, check consistency with the brief, identify ambiguous "
                  "areas or conflicts between mockups and constraints. Run a review session with the team and "
                  "client to validate before sign-off.",
                  "in_progress", order++, "project-designer")) return 1;
    std::string t4 = generate_uuid_v4();
    if (!task_add(db, t4, "P1", "M1", "Draft product catalogue structure",
                  "Define the product catalogue structure: category hierarchy, attributes (name, price, stock, "
                  "SKU, images), search facets and filters. Define SEO slugs and alignment constraints with "
                  "inventory and CMS. Plan multi-language evolution if applicable.",
                  "done", order++, "project-designer")) return 1;
    std::string t5 = generate_uuid_v4();
    if (!task_add(db, t5, "P1", "M1", "User flows for checkout",
                  "Design user flows for the purchase funnel: add to cart, update quantities, promo codes, "
                  "shipping and payment choice. Include error cases (out of stock, payment declined) and "
                  "intermediate states (abandoned cart, session recovery).",
                  "done", order++, "ux-designer")) return 1;
    std::string t6 = generate_uuid_v4();
    if (!task_add(db, t6, "P1", "M1", "Prioritise MVP scope with client",
                  "Facilitate a prioritisation session with the client: feature backlog, voting or MoSCoW, "
                  "trade-offs under time or budget constraints. Lock the MVP scope and list features deferred "
                  "to phase 2.",
                  "to_do", order++, std::nullopt)) return 1; // unassigned

    // --- P2 / M2 (Development — MVP): 14 tasks (incl. one without milestone, one very long title)
    std::string t7 = generate_uuid_v4();
    if (!task_add(db, t7, "P2", "M2", "Implement auth module (API)",
                  "Implement authentication endpoints (login, logout, refresh, revoke) per spec. Handle rate "
                  "limiting, audit logging of login attempts and token expiration. Document the API in OpenAPI "
                  "and provide unit and integration tests.",
                  "in_progress", order++, "developer")) return 1;
    std::string t8 = generate_uuid_v4();
    if (!task_add(db, t8, "P2", "M2", "Implement login screen",
                  "Build the login screen (email/password) and integration with the auth API: error handling "
                  "(wrong credentials, disabled account), \"forgot password\" flow, \"remember me\" and "
                  "post-login redirect by role.",
                  "to_do", order++, "developer")) return 1;
    std::string t9 = generate_uuid_v4();
    if (!task_add(db, t9, "P2", "M2", "Implement product listing and search",
                  "Implement product listing and search: pagination, sort (price, new, relevance), filters by "
                  "category and attributes, empty results and loading states. Respect catalogue structure and "
                  "performance constraints (indexes, cache where needed).",
                  "to_do", order++, "developer")) return 1;
    std::string t10 = generate_uuid_v4();
    if (!task_add(db, t10, "P2", "M2", "Implement cart (add/remove/update quantities)",
                  "Implement cart on front and back: add/remove items, update quantities, persistence (session "
                  "or account), total recalculation and real-time stock checks. Handle conflicts (stock depleted "
                  "between add and checkout).",
                  "to_do", order++, "developer")) return 1;
    std::string t11 = generate_uuid_v4();
    if (!task_add(db, t11, "P2", "M2",
                  "Implement Stripe payment tunnel integration including webhook handling, retry logic, idempotency keys, and frontend-backend contract alignment (PCI-DSS scope)",
                  "Integrate the Stripe payment flow: create PaymentIntents, handle webhooks "
                  "(payment_intent.succeeded, failed), idempotency keys and retry logic. Align front/back contract "
                  "and stay within the defined PCI-DSS scope (no card number storage).",
                  "to_do", order++, "developer")) return 1; // very long title — UI stress test
    std::string t12 = generate_uuid_v4();
    if (!task_add(db, t12, "P2", "M2", "Refactor shopping cart to support multi-rule promotions, stackable promo codes, and A/B testing of discount offers (API + cache invalidation strategy)",
                  "Refactor the cart to support multiple promotion rules (percentage, fixed amount, buy-N), "
                  "stackable or exclusive promo codes, and A/B testing of offers. Define cache invalidation "
                  "strategy and related API endpoints.",
                  "to_do", order++, std::nullopt)) return 1; // unassigned + long title
    std::string t13 = generate_uuid_v4();
    if (!task_add(db, t13, "P2", "M2", "Document auth API",
                  "Write developer-facing documentation: auth endpoint descriptions, request/response schemas, "
                  "code samples (cURL or SDK), error handling and best practices (refresh token, client-side "
                  "security). Update the README or technical docs.",
                  "to_do", order++, "documentation-writer")) return 1;
    std::string t14 = generate_uuid_v4();
    if (!task_add(db, t14, "P2", "M2", "Setup CI pipeline and branch strategy",
                  "Configure the CI pipeline: build on every push, run linters and unit tests, protect main "
                  "(mandatory review, green status). Define branch strategy (feature, release, hotfix) and "
                  "document it in the repo README.",
                  "to_do", order++, "devops-engineer")) return 1;
    std::string t15 = generate_uuid_v4();
    if (!task_add(db, t15, "P2", std::nullopt, "Technical spike: evaluate Stripe vs PayPal",
                  "Technical spike to compare Stripe and PayPal: pricing, integration (API, SDK), geographic "
                  "coverage, webhook and dispute handling. Produce a summary with a recommendation and "
                  "integration plan for the chosen solution.",
                  "done", order++, "software-architect")) return 1; // no milestone
    std::string t16 = generate_uuid_v4();
    if (!task_add(db, t16, "P2", "M2", "Implement order confirmation email",
                  "Set up order confirmation email after successful payment: HTML/text template, dynamic data "
                  "(order number, summary, shipping address), SMTP or provider config (SendGrid, etc.) and "
                  "handling of send failures.",
                  "to_do", order++, std::nullopt)) return 1; // unassigned
    std::string t17 = generate_uuid_v4();
    if (!task_add(db, t17, "P2", "M2", "Secure admin endpoints and rate limiting",
                  "Secure admin endpoints: RBAC (roles and permissions), logging of sensitive actions, rate "
                  "limiting to prevent abuse. Align with the security review checklist and prepare documentation "
                  "for audit.",
                  "to_do", order++, "security-engineer")) return 1;
    std::string t18 = generate_uuid_v4();
    if (!task_add(db, t18, "P2", "M2", "Style catalogue and cart (responsive)",
                  "Apply the design system to catalogue and cart: responsive layout (mobile, tablet, desktop), "
                  "typography, colours and components (buttons, product cards). Ensure consistency with "
                  "mockups and correct rendering in major browsers.",
                  "to_do", order++, "ui-designer")) return 1;
    std::string t19 = generate_uuid_v4();
    if (!task_add(db, t19, "P2", "M2", "Review accessibility (keyboard, screen reader)",
                  "Verify site accessibility: keyboard navigation (tab order, visible focus), screen reader "
                  "support (ARIA, labels), text contrast and alternatives for visual content. Use tools "
                  "(axe, Lighthouse) and fix identified issues.",
                  "to_do", order++, "qa-engineer")) return 1;
    std::string t20 = generate_uuid_v4();
    if (!task_add(db, t20, "P2", "M2", "Wire payment success/error to order status and notifications",
                  "Connect payment flow outcomes: on success, update order status, trigger confirmation email "
                  "and any push or in-app notifications. On error, show a clear message and offer retry or "
                  "alternative payment.",
                  "to_do", order++, "developer")) return 1;

    // --- P3 / M3 (Acceptance): 6 tasks
    std::string t21 = generate_uuid_v4();
    if (!task_add(db, t21, "P3", "M3", "Write E2E tests for login",
                  "Write end-to-end tests for the login flow: valid form, wrong credentials, disabled account, "
                  "logout and session persistence. Use an E2E framework (Playwright, Cypress) and dedicated "
                  "test accounts.",
                  "to_do", order++, "developer")) return 1;
    std::string t22 = generate_uuid_v4();
    if (!task_add(db, t22, "P3", "M3", "E2E tests for checkout and payment",
                  "E2E coverage of checkout and payment: happy path, declined card, payment timeout, webhook "
                  "replay. Use Stripe test-mode fixtures and reproducible datasets. Verify order status update "
                  "and confirmation email sending.",
                  "to_do", order++, "qa-engineer")) return 1;
    std::string t23 = generate_uuid_v4();
    if (!task_add(db, t23, "P3", "M3", "Run regression on catalogue and cart",
                  "Run the regression suite on catalogue and cart: listing, search, filters, add/remove from "
                  "cart, totals, promo codes. Compare with the latest baseline and report any functional or "
                  "performance regression.",
                  "to_do", order++, "qa-engineer")) return 1;
    std::string t24 = generate_uuid_v4();
    if (!task_add(db, t24, "P3", "M3", "Fix blocking bugs from acceptance",
                  "Fix blocking bugs found in acceptance testing: prioritise with the product owner, assign "
                  "fixes, re-test after each fix and update tickets until delivery criteria are met.",
                  "to_do", order++, std::nullopt)) return 1; // unassigned
    std::string t25 = generate_uuid_v4();
    if (!task_add(db, t25, "P3", "M3", "Sign-off with product owner",
                  "Run the acceptance demo with the product owner: walk through MVP user stories, validate "
                  "acceptance criteria and get formal sign-off for delivery. Document any reservations or "
                  "improvements deferred to phase 2.",
                  "to_do", order++, "product-owner")) return 1;
    std::string t26 = generate_uuid_v4();
    if (!task_add(db, t26, "P3", "M3", "Update release notes",
                  "Write or update release notes for the version: new features, bug fixes and notable technical "
                  "changes. Adapt tone for the audience (end users or internal) and have them reviewed before "
                  "publish.",
                  "to_do", order++, "summary-writer")) return 1;

    // --- P4 / M4 (Delivery): 5 tasks
    std::string t27 = generate_uuid_v4();
    if (!task_add(db, t27, "P4", "M4", "Write deployment runbook",
                  "Write the deployment runbook: build steps, secrets and environment variables, database "
                  "migrations, health check verification and rollback procedure. Include contacts and "
                  "escalation chain for incidents.",
                  "to_do", order++, "documentation-writer")) return 1;
    std::string t28 = generate_uuid_v4();
    if (!task_add(db, t28, "P4", "M4", "Deploy to staging and smoke tests",
                  "Deploy the application to the staging environment per runbook, run migrations and smoke tests "
                  "(login, catalogue, cart, test payment). Check logs and metrics, confirm the environment "
                  "is stable before production.",
                  "to_do", order++, "devops-engineer")) return 1;
    std::string t29 = generate_uuid_v4();
    if (!task_add(db, t29, "P4", "M4", "Deploy to production",
                  "Deploy to production at the agreed time: follow the runbook, monitor health checks and "
                  "metrics, inform stakeholders. On failure, trigger rollback and communication per procedure.",
                  "to_do", order++, "project-manager")) return 1;
    std::string t30 = generate_uuid_v4();
    if (!task_add(db, t30, "P4", "M4", "Configure monitoring and alerts",
                  "Configure monitoring and alerts: application and infra metrics, thresholds (latency, "
                  "error rate, availability), notification channels (email, Slack, PagerDuty). Define "
                  "dashboards and document runbooks for common alerts.",
                  "to_do", order++, "devops-engineer")) return 1;
    std::string t31 = generate_uuid_v4();
    if (!task_add(db, t31, "P4", "M4", "Handover and retrospective",
                  "Conduct handover with support/maintenance: deliverables, access, documentation and "
                  "contacts. Then run the project retrospective: what went well, improvement areas and "
                  "lessons for future projects.",
                  "to_do", order++, std::nullopt)) return 1; // unassigned

    std::cout << "  tasks 1-" << (order - 1) << " (P1: 6, P2: 14, P3: 6, P4: 5)\n";

    // Dependencies (DAG): design -> dev -> acceptance -> delivery; cross-links where logical
    if (!task_dep_add(db, t7, t2)) return 1;   // auth API impl depends on auth spec
    if (!task_dep_add(db, t8, t7)) return 1;  // login screen after auth API
    if (!task_dep_add(db, t9, t4)) return 1;  // listing after catalogue structure
    if (!task_dep_add(db, t10, t9)) return 1; // cart after listing
    if (!task_dep_add(db, t11, t15)) return 1; // Stripe impl after spike
    if (!task_dep_add(db, t11, t10)) return 1; // payment after cart
    if (!task_dep_add(db, t12, t10)) return 1; // cart refactor after cart
    if (!task_dep_add(db, t13, t7)) return 1; // doc auth after auth API
    if (!task_dep_add(db, t20, t11)) return 1; // wire payment UI after Stripe
    if (!task_dep_add(db, t21, t8)) return 1; // E2E login after login screen
    if (!task_dep_add(db, t22, t20)) return 1; // E2E payment after wired flow
    if (!task_dep_add(db, t23, t10)) return 1; // regression cart
    if (!task_dep_add(db, t25, t22)) return 1; // sign-off after E2E payment
    if (!task_dep_add(db, t25, t23)) return 1; // sign-off after regression
    if (!task_dep_add(db, t26, t25)) return 1; // release notes after sign-off
    if (!task_dep_add(db, t28, t26)) return 1; // deploy staging after release notes
    if (!task_dep_add(db, t29, t28)) return 1; // prod after staging
    if (!task_dep_add(db, t29, t27)) return 1; // prod after runbook
    if (!task_dep_add(db, t31, t29)) return 1; // handover after prod
    std::cout << "  dependencies\n";

    // Notes (completion / progress) on a few tasks
    if (!note_add(db, generate_uuid_v4(), t1, "Requirements doc approved by client on review. Scope locked for MVP.", "completion", "project-manager")) return 1;
    if (!note_add(db, generate_uuid_v4(), t2, "OpenAPI spec published. SSO deferred to phase 2.", "completion", "software-architect")) return 1;
    if (!note_add(db, generate_uuid_v4(), t3, "Review session scheduled. One open point on error-message wording.", "progress", "project-designer")) return 1;
    if (!note_add(db, generate_uuid_v4(), t4, "Structure validated with inventory team. Multi-language fields added for later.", "completion", "project-designer")) return 1;
    if (!note_add(db, generate_uuid_v4(), t5, "Flows signed off. Abandoned-cart recovery moved to phase 2.", "completion", "ux-designer")) return 1;
    if (!note_add(db, generate_uuid_v4(), t7, "Login and refresh implemented. Revoke endpoint in progress.", "progress", "developer")) return 1;
    if (!note_add(db, generate_uuid_v4(), t15, "Stripe chosen: better docs and webhook UX. Recommendation doc in Confluence.", "completion", "software-architect")) return 1;
    std::cout << "  notes (7)\n";

    std::cout << "Done. Database: " << db_path << "\n";
    return 0;
}

} // namespace taskman
