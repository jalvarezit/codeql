private import codeql.ruby.AST
private import codeql.ruby.Concepts
private import codeql.ruby.DataFlow
private import codeql.ruby.frameworks.Core

module BasicObject {
  /**
   * An instance method on `BasicObject`, which is available to all classes.
   */
  class BasicObjectInstanceMethodCall extends UnknownMethodCall {
    BasicObjectInstanceMethodCall() { this.getMethodName() = basicObjectInstanceMethodName() }
  }

  string basicObjectInstanceMethodName() {
    result in [
        "equal?", "instance_eval", "instance_exec", "method_missing", "singleton_method_added",
        "singleton_method_removed", "singleton_method_undefined"
      ]
  }

  /**
   * A call to `BasicObject#instance_eval`, which executes its first argument as Ruby code.
   */
  class InstanceEvalCallCodeExecution extends CodeExecution::Range, DataFlow::CallNode {
    InstanceEvalCallCodeExecution() {
      this.asExpr().getExpr().(UnknownMethodCall).getMethodName() = "instance_eval"
    }

    override DataFlow::Node getCode() { result = this.getArgument(0) }
  }
}
