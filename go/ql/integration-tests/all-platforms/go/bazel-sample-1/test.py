import os
import subprocess

from create_database_utils import *
from diagnostics_test_utils import *

# Set up a GOPATH relative to this test's root directory;
# we set os.environ instead of using extra_env because we
# need it to be set for the call to "go clean -modcache" later
goPath = os.path.join(os.path.abspath(os.getcwd()), ".go")
os.environ['GOPATH'] = goPath

run_codeql_database_create([], lang="go", source="src")

check_diagnostics()

# Clean up the temporary GOPATH
subprocess.call(["go", "clean", "-modcache"])
