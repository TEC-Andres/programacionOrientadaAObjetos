# git policies

## module description
this document outlines the source tree workflow requirements and repository branching models that developers must follow to keep development histories orderly.

## branch naming system
branch naming must strictly use the following taxonomy keywords:
* main - reserved for production releases.
* dev - integration branch for consolidation testing routines.
* feature/<name> - isolated workspaces for engineering updates.
* bugfix/<name> - hotfix lines for eliminating execution bugs.
* docs/<name> - paths for updating project documentation materials.
* translation/<name> - localization scopes for internationalization updates.
* structure/<name> - workspaces dedicated to file reorganizations.