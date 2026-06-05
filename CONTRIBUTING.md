# Contribution Nomenclature
## General Rules
### Pull requests (PR)
1.- All contributions must be made via PRs in the GitHub repository.  
2.- Each PR must include a clear description of the changes made and the reason for those changes.  
3.- PRs must pass all tests before being reviewed and approved.  
4.- Each PR must be verified before being merged into main.  

### Issues
1.- We will use issues to create tasks and report bugs.  
2.- Each issue must have a clear description of the problem or task to be done.  
3.- Each issue must be labeled correctly to make identification and tracking easier.  
4.- Issues must be assigned to team members responsible for their resolution.  
5.- If an issue is not resolved on time, it should be communicated to the team to reassign or adjust the resolution time.  

### Code
1.- Code must follow the style conventions established by the C++ committee.  
2.- The functionality of each function must be documented as follows:  
```cpp
/**
 * @brief Brief description of the function.
 *
 * Detailed description of the function, including its parameters and return value.
 *
 * @param param1 Description of the first parameter.
 * @param param2 Description of the second parameter.
 * @return Description of the return value.
 */
void exampleFunction(int param1, std::string param2);
```
3.- Code must be modular and reusable, avoiding duplication.  
4.- Code must be tested and documented before being merged into main.  

### Docs
1.- Within the repository, there's going to be a docs section where all documentation related to the project will be stored.  
2.- Documentation must be clear, concise, and up-to-date.  
3.- Documentation must include installation instructions, usage guides, and API references.  

## Branching Strategy
We will follow the following nomenclature for branches:
- `main`: This branch will contain the stable version of the code. All changes must be merged into this branch after being reviewed and approved.
- `dev`: This is a branch to test everything before merging into main. All new features and bug fixes must be merged into this branch first for testing.
- `feature/feature-name`: This branch will be used for developing new features. Each feature must have its own branch, which will be merged into dev once the feature is complete and tested.
- `bugfix/bug-name`: This branch will be used for fixing bugs. Each bug fix must have its own branch, which will be merged into dev once the fix is complete and tested.
- `docs/doc-name`: This branch will be used for updating documentation. Each documentation update must have its own branch, which will be merged into dev once the update is complete and reviewed.
- `structure/structure-name`: This branch will be used for making structural changes to the codebase, such as refactoring or reorganizing files. Each structural change must have its own branch, which will be merged into dev once the change is complete and tested.

> [!WARNING]
> Branches must be named according to the established nomenclature. Failure to do so will result in a rejected branch until named correctly.