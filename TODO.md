# To dos

- Add unit tests for assembly and C programs.
- Add multiply and divide instructions.
- Add CSR instructions.
- Extend the architectural test coverage.
  * Update rvsim_isa.yaml as extensions are added, so RISCOF selects the tests
    that cover them.
  * Implement traps, so the misaligned-access tests can check that an exception
    is raised rather than that the access succeeds.
  * Move to the ACT 4.0 framework, which replaces RISCOF and needs the Sail
    reference model and a UDB configuration.
- Setup github actions CI.
