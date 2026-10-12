#ifndef CKLIB_CK_PERFORMANCE_METRICS_H_
#define CKLIB_CK_PERFORMANCE_METRICS_H_

#include "cklib/inc/ck_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// A struct to hold all calculated metrics.
// It is recommended to initialize this struct to zero before passing it.
typedef struct {
    double y_stable;            // The calculated steady-state value.
    double y_peak;              // The peak value of the response.
    double settling_time;       // Settling Time (Ts) in seconds.
    double peak_time;           // Peak Time (Tp) in seconds.
    double maximum_overshoot;   // Maximum Overshoot (Mp) as a percentage.
    double rise_time;           // Rise Time (Tr, 10% to 90%) in seconds.
    double delay_time;          // Delay Time (Td, 50%) in seconds.
    double steady_state_error;  // Absolute Steady-State Error (Ess).
} PerformanceMetrics;

/**
 * @brief (Adaptive Algorithm) Calculates settling time and the steady-state value.
 * @param data Pointer to the complete response data array.
 * @param data_cnt Total number of data points.
 * @param setpoint The target setpoint value.
 * @param dt The sampling time interval in seconds.
 * @param percentage The tolerance percentage for the settling band (e.g., 2.0 for ±2%).
 * @param[out] ts Pointer to store the calculated settling time.
 * @param[out] y_stable Pointer to store the calculated steady-state value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckCalcSettlingTime(const double *data,
                                     int data_cnt,
                                     double setpoint,
                                     double dt,
                                     double percentage,
                                     double *ts,
                                     double *y_stable);

/**
 * @brief Finds the absolute peak value and the time at which it occurs.
 * @param data Pointer to the complete response data array.
 * @param data_cnt Total number of data points.
 * @param dt The sampling time interval in seconds.
 * @param[out] tp Pointer to store the peak time.
 * @param[out] y_peak Pointer to store the peak value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckCalcPeakTime(const double *data,
                                 int data_cnt,
                                 double dt,
                                 double *tp,
                                 double *y_peak);

/**
 * @brief Calculates the maximum overshoot based on pre-calculated peak and stable values.
 * @param data Pointer to the complete response data array.
 * @param data_cnt Total number of data points.
 * @param y_stable The final steady-state value of the response.
 * @param y_peak The peak value of the response.
 * @param[out] mo Pointer to store the maximum overshoot percentage.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR ckCalcMaximumOvershoot(const double *data,
                                int data_cnt,
                                double y_stable,
                                double y_peak,
                                double *mo);

/**
 * @brief Calculates the rise time (10% to 90%) of the response.
 * @param data Pointer to the complete response data array.
 * @param data_cnt Total number of data points.
 * @param y_stable The final steady-state value of the response.
 * @param dt The sampling time interval in seconds.
 * @param[out] tr Pointer to store the rise time.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckCalcRiseTime(const double *data,
                                 int data_cnt,
                                 double y_stable,
                                 double dt,
                                 double *tr);

/**
 * @brief Calculates the delay time (50%) of the response.
 * @param data Pointer to the complete response data array.
 * @param data_cnt Total number of data points.
 * @param y_stable The final steady-state value of the response.
 * @param dt The sampling time interval in seconds.
 * @param[out] td Pointer to store the delay time.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckCalcDelayTime(const double *data,
                                  int data_cnt,
                                  double y_stable,
                                  double dt,
                                  double *td);

/**
 * @brief Calculates the steady-state error.
 * @param y_stable The final steady-state value of the response.
 * @param setpoint The target setpoint value.
 * @param[out] ess Pointer to store the absolute steady-state error.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckCalcSteadyStateError(double y_stable,
                                         double setpoint,
                                         double *ess);

/**
 * @brief Calculates all step response metrics and populates the metrics struct.
 * This is the recommended function for general use.
 * @param data Pointer to the complete response data array.
 * @param data_cnt Total number of data points.
 * @param setpoint The target setpoint value.
 * @param dt The sampling time interval in seconds.
 * @param settling_percentage The tolerance percentage for settling time.
 * @param[out] metrics Pointer to the PerformanceMetrics struct to be filled.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckCalcPerformanceMetrics(const double *data,
                                           int data_cnt,
                                           double setpoint,
                                           double dt,
                                           double settling_percentage,
                                           PerformanceMetrics *metrics);

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_CK_PERFORMANCE_METRICS_H_
