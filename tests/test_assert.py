import pathlib
import subprocess

THIS_DIR = pathlib.Path(__file__).parent
ASSERT_EXECUTABLE = str(THIS_DIR / "assert")


def test_assert():
    assert subprocess.run(ASSERT_EXECUTABLE).returncode == 0
