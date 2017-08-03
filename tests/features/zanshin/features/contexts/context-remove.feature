Feature: Context removal
  As someone using tasks
  I can remove a context
  In order to maintain their semantic

  Scenario: Removed context disappear from the list
    Given I display the available pages
    When I remove the page named "Online" under "Contexts"
    And I list the items
    Then the list is:
       | display                                                | icon              |
       | Inbox                                                  | mail-folder-inbox |
       | Workday                                                | go-jump-today     |
       | Projects                                               | folder            |
       | Projects / TestData » Calendar1                          | folder            |
       | Projects / TestData » Calendar1 / Prepare talk about TDD | view-pim-tasks    |
       | Projects / TestData » Calendar1 / Read List              | view-pim-tasks    |
       | Projects / TestData » Calendar1 » Calendar2                | folder            |
       | Projects / TestData » Calendar1 » Calendar2 / Backlog      | view-pim-tasks    |
       | Contexts                                               | folder            |
       | Contexts / Errands                                     | view-pim-notes    |

