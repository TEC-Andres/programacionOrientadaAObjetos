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