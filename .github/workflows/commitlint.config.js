module.exports = {
  extends: ["@commitlint/config-conventional"],
  rules: {
    "header-max-length": () => [2, "always", 72],
    "type-enum": () => [
      2,
      "always",
      ["feat", "fix", "refactor", "chore", "test", "doc", "release", "revert"],
    ],
    "scope-enum": [
      2,
      "always",
      ["core", "database", "functions", "storage", "test"],
    ],
    "body-max-line-length": [0, "always"],
    "footer-max-line-length": [0, "always"],
  },
  helpUrl:
    "https://github.com/flutter-tizen/flutterfire/blob/main/CONTRIBUTING.md",
};
