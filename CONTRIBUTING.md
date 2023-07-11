# Contributing to FlutterFire for Tizen

## Commit message guide

### Guides and references

Commit message guidelines are based on the following guides (highest priority
first):

1. This document.
2. The [Conventional Commits].

#### Formatting

```
<type>(<scope>): <subject>

<body>

<footer>
```

`(<scope>)`, `<body>`, and `<footer>` are optional. The first line is considered
as a header which should:

- not be longer than 72 characters.
- be prefixed with the header format.
- start with an imperative verb following the prefix.

##### Examples:

- `feat(core): add handling error result`
- `chore(release): update dependencies version`
- `doc: update LICENSE`

##### Example `<type>` values:

```
feat: (new feature)
fix: (bug fix)
refactor: (refactoring the code including styling)
chore: (updating build tasks, release, scriptings, etc.)
test: (adding tests, refactoring tests, etc.)
doc: (changes to the documentation)
```

##### Example `<scope>` values:

```
core
database
functions
storage
release
etc.
```

## C++ style guide

Sources located under `tizen/inc` and `tizen/src` must comply with this guide.

### Guides and references

Coding guidelines are based on the following guides (highest priority first):

1. This document.
2. The [Google C++ Style Guide][].
3. The ISO [C++ Core Guidelines][].

#### Formatting

- 2 spaces of indentation for blocks or bodies of conditionals
- CamelCase for methods, functions, and classes
- snake_case for local variables and parameters
- snake_case_ for private class fields
- snake_case for C-like structs

[Conventional Commits]: https://www.conventionalcommits.org/
[Google C++ Style Guide]: https://google.github.io/styleguide/cppguide.html
[C++ Core Guidelines]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

## Landing guide

These guidelines ensure proper review, approval, and testing of pull requests
before merging into the codebase.

### Pull request requirements
- A pull request must pass CI (Continuous Integration) tests run.
- A pull request must be reviewed and approved by at least two committers.
- If a pull request has been open for more than 7 days with no objection, one
  approval is enough.


### Duration of pull request
- Recommended: Keep a pull request open for 24 hours even after approval and
  passing the CI. This allows time for review feedback.
- Committers may skip waiting if changes are trivial.
