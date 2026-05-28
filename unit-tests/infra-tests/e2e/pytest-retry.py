# Verify native pytest-retry behaviour with --retries: a flaky test that
# fails attempt 1 and passes attempt 2 should be reported as a single PASS.
# Module-scoped fixtures are torn down + re-created between attempts, which
# is how device recycling and precondition re-apply happen automatically.
import pytest

pytestmark = [pytest.mark.device("D455")]

_fail_attempt = 0

def test_always_passes(module_device_setup):
    """Always passes - module fixture stays alive across this test."""
    pass

def test_fails_then_passes(module_device_setup):
    """Fail on attempt 1, pass on attempt 2 (after pytest-retry tears down +
    re-creates module fixtures via its preliminary teardown trick)."""
    global _fail_attempt
    _fail_attempt += 1
    if _fail_attempt == 1:
        assert False, "intentional first-attempt failure"
