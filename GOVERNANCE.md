# Contributors & Governance

## Core Team

This project is maintained by a small group of robotics engineers dedicated to open-source contribution.

* **Woojin Jung (@frozenreboot)**
    * **Role:** Lead Maintainer / Author
    * **Responsibilities:** Core implementation, Release decisions, Roadmap definition, Issue tracking.

* **Orhan G. Hafif (@cosmicog)**
    * **Role:** Co-Maintainer / Technical Advisor
    * **Responsibilities:** Architecture review, Industrial use-case validation, Edge case analysis, Code review.

* **Błażej Sowa (@bjsowa)**
    * **Role:** Release Maintainer / Packaging Maintainer
    * **Responsibilities:** rosdistro release process, bloom workflow, release repository management (ros2-gbp), packaging conventions, changelog format.

## Collaboration Protocol

We value **async communication** and **respect for time**. Since maintainers have full-time commitments in the industry, we follow a flexible workflow:

### 1. Release Cycle
* **Development:** Features are primarily developed on a weekly basis.
* **Pull Requests:** PRs are typically opened during the weekend.
* **Review Window:** We aim for a "Best Effort" review process.

### 2. "Lazy Consensus" for Merging
To prevent blocking development due to busy schedules:
* Maintainers will request a review for major changes.
* If no critical objections or reviews are posted within **7 days** (or sooner for hotfixes), the Lead Maintainer may proceed to merge, assuming implied consent.
* Of course, critical architectural changes will wait for a thorough discussion.

### 3. Review Focus
* We focus on **Logic, Safety, and Stability** rather than nitpicking style (Linting is handled by CI).
* **Industrial Rationale:** We highly value feedback based on real-world industrial environments (e.g., sensor noise, hardware quirks).
