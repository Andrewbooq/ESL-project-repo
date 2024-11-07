# The common rules for the current repo

## 1 Tasks and projects
- All work shold be under tasks in [ESL-project](https://github.com/users/Andrewbooq/projects/2).

## 2 Branches and tags
- A branch for a new workshop should have prefix feature/ and includes Workshop number (feature/WS4_pwm_and_double_click).
- A branch for a bug fix should have prefix bug/ (bug/WS4_cover_debouncing_all_cases).
- After brunch is done (all commits are pushed) it's necessary to Merge the branch to master over Pull Request.
- After the branch is merged to master it's necessary to create a tag with name that is the same to the branch but without any prefix (WS4_pwm_and_double_click).
- 
## 3 Comments
- Comments to Pull Requests should be multiline. First line inludes the workshop number and description (Workshop3: Use GPIO), next line - empty, next line refers to a task number in thr project (Related to #17):
  ```
  Workshop3: Use GPIO
  
  Related to #17
  ```


