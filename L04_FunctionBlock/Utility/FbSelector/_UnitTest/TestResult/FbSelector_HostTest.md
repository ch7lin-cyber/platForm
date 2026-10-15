# FbSelector host unit test

- Date: 2026-10-16
- Result: PASS
- Build: `cc -std=c99 -Wall -Wextra -Werror -pedantic`
- Coverage: indexed selection, minimum and maximum selection, tie-breaking to the lowest index, active input count, ignored unused slots, invalid mode/index/count, NaN/Infinity selected inputs, disabled state, reset, and null pointers.
- Execution: standalone host test returned exit code 0.
