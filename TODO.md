# To dos

- Add unit tests for assembly and C programs.
- Add multiply and divide instructions.
- Extend the architectural test coverage.
  * Update rvsim_isa.yaml and tests/act4/rvsim/rvsim.yaml as extensions are
    added, so the tests that cover them are selected.
  * Run the ACT 4.0 flow end to end, which needs GCC 15 and Spike, and fix up
    the DUT macros in tests/act4/rvsim/rvmodel_macros.h from what it reports.
  * Implement the machine timer and interrupts, so the timer and interrupt
    tests can be enabled.
  * Turn on the ACT 4.0 privileged tests, which need PMP, counters and
    delegation.
- Setup github actions CI.
