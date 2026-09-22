# Standard sensor tables

Each sensor type owns one `<sensor>_table.c/.h` pair. Conversion code
remains in PlatformCore; this directory contains reusable standard coefficient
data only.

## Naming

- Measurement: `<Sensor>Table_GetMeasurementTable()`
- Cold junction: `<Sensor>Table_GetCjcTable()` (thermocouples only)
- Readiness: `<Sensor>Table_IsReady()`

## Fixed-point row

```c
{x_min, x_max, slope_q, intercept_q}
```

```text
y = (slope_q * x + intercept_q) / coefficient_scale
```

Units:

| Table | x | y |
|---|---|---|
| TC measurement | uV | 0.001 degree C |
| TC CJC | 0.001 degree C | uV |
| RTD measurement | milliohm | 0.001 degree C |

Adjacent rows must be monotonic and cover their boundary without a gap.
Incomplete tables must return `NULL` and must not be registered for runtime use.

## Thermocouple table status

| Type | File stem | Rated range (degree C) | Status |
|---|---|---:|---|
| K | `tc_k_table` | -200 to 1300 | Complete |
| J | `tc_j_table` | -200 to 1200 | Complete |
| T | `tc_t_table` | -200 to 400 | Complete |
| E | `tc_e_table` | 0 to 600 | Complete |
| N | `tc_n_table` | -200 to 1300 | Complete |
| R | `tc_r_table` | 0 to 1700 | Complete |
| S | `tc_s_table` | 0 to 1700 | Complete |
| B | `tc_b_table` | 100 to 1800 | Data requires review |
| L | `tc_l_table` | -200 to 850 | Complete |
| U | `tc_u_table` | -200 to 500 | Complete |
| TXK | `tc_txk_table` | -150 to 800 | Complete |
| C | `tc_c_table` | 0 to 2300 | Complete |
| D | `tc_d_table` | 0 to 2300 | Complete |

When replacing any TC data, set `TC_<TYPE>_TABLE_COMPLETE` to `0U`
while editing and restore it to `1U` only after validation.

## RTD table status

| Type | File stem | Rated range (degree C) | Status |
|---|---|---:|---|
| Pt100 | `rtd_pt100_table` | -200 to 850 | Complete |
| JPt100 | `rtd_jpt100_table` | -20 to 400 | Complete |
| Ni120 | `rtd_ni120_table` | -80 to 300 | Complete |
| Cu50 | `rtd_cu50_table` | -50 to 150 | Complete |
| Pt1000 | `rtd_pt1000_table` | -200 to 850 | Complete |

RTD table input is calibrated resistance in milliohm. Its coefficient input is
shifted before evaluating `y = ax + b`:

```text
shifted_resistance_milliohm = resistance_milliohm + input_shift_milliohm
temperature_mC = slope * shifted_resistance_milliohm / coefficient_scale
                 + intercept_mC
```

When replacing any RTD data, set `RTD_<TYPE>_TABLE_COMPLETE` to `0U`
while editing and restore it to `1U` only after validating the segment count,
boundaries, coefficients and `INPUT_SHIFT_MILLIOHM`.

## Validation notes

- Every table must contain exactly `segment_count + 1` boundaries and
  `segment_count` coefficient rows.
- Boundaries must be strictly increasing for binary-search evaluation.
- TC-B currently starts with `98, 100, 97, 100`; this is not strictly
  increasing, so `TcBTable_IsReady()` returns `false`. Confirm and
  regenerate the low-temperature B-type input data before runtime use.
