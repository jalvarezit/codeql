/** Provides a taint-tracking configuration to reason about improper validation of user-provided size used for array construction. */

import java
import semmle.code.java.security.internal.ArraySizing
import semmle.code.java.dataflow.FlowSources

/**
 * A taint-tracking configuration to reason about improper validation of user-provided size used for array construction.
 */
private module ImproperValidationOfArrayConstructionConfig implements DataFlow::ConfigSig {
  predicate isSource(DataFlow::Node source) { source instanceof RemoteFlowSource }

  predicate isSink(DataFlow::Node sink) {
    any(CheckableArrayAccess caa).canThrowOutOfBoundsDueToEmptyArray(sink.asExpr(), _)
  }
}

/**
 * Taint-tracking flow for improper validation of user-provided size used for array construction.
 */
module ImproperValidationOfArrayConstructionFlow =
  TaintTracking::Global<ImproperValidationOfArrayConstructionConfig>;
