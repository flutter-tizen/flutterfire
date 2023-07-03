# Contributing to FlutterFire for Tizen

## Commit message guidelines

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
